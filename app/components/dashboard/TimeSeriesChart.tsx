"use client";

// Gráfico de linha para os datasets do backend ({labels, series[], threshold?}).
// Cores por índice fixo (--chart-1..3, validadas p/ CVD/contraste nos dois temas);
// texto sempre em tokens de texto, nunca na cor da série.

import {
  CartesianGrid,
  Line,
  LineChart,
  ReferenceLine,
  ResponsiveContainer,
  Tooltip,
  XAxis,
  YAxis,
  type TooltipContentProps,
} from "recharts";
import type { ChartDataset } from "@/lib/types";

const SERIES_COLORS = ["var(--chart-1)", "var(--chart-2)", "var(--chart-3)"];

export type TimeSeriesChartProps = {
  dataset: ChartDataset;
  /** Altura da área de plot, em px. */
  height?: number;
  /** Domínio fixo do eixo Y (ex.: [0, 120] para percentual). */
  yDomain?: [number, number | "auto"];
  /** Sufixo dos valores no tooltip/eixo (ex.: "%"). */
  valueSuffix?: string;
};

/** Timestamps MySQL "YYYY-MM-DD HH:MM:SS" → Date (parse local). */
function parseLabel(label: string): Date | null {
  const date = new Date(label.replace(" ", "T"));
  return Number.isNaN(date.getTime()) ? null : date;
}

function formatTick(label: string, withDate: boolean): string {
  const date = parseLabel(label);
  if (!date) return label;
  const time = date.toLocaleTimeString("pt-BR", { hour: "2-digit", minute: "2-digit" });
  if (!withDate) return time;
  const day = date.toLocaleDateString("pt-BR", { day: "2-digit", month: "2-digit" });
  return `${day} ${time}`;
}

function formatFullLabel(label: string): string {
  const date = parseLabel(label);
  if (!date) return label;
  return date.toLocaleString("pt-BR", {
    day: "2-digit",
    month: "2-digit",
    hour: "2-digit",
    minute: "2-digit",
    second: "2-digit",
  });
}

function ChartTooltip({
  active,
  payload,
  label,
  valueSuffix,
}: TooltipContentProps & { valueSuffix?: string }) {
  if (!active || !payload || payload.length === 0) return null;

  return (
    <div className="rounded-lg border border-border bg-card px-3 py-2 text-sm shadow-sm">
      <p className="mb-1 text-xs text-muted-foreground">{formatFullLabel(String(label))}</p>
      <ul className="flex flex-col gap-0.5">
        {payload.map((entry) => (
          <li key={String(entry.dataKey)} className="flex items-center gap-2 text-foreground">
            <span
              aria-hidden="true"
              className="inline-block h-2.5 w-2.5 rounded-full"
              style={{ background: entry.color }}
            />
            <span className="text-muted-foreground">{entry.name}:</span>
            <span className="font-medium tabular-nums">
              {typeof entry.value === "number" ? entry.value.toLocaleString("pt-BR") : entry.value}
              {valueSuffix ?? ""}
            </span>
          </li>
        ))}
      </ul>
    </div>
  );
}

export function TimeSeriesChart({ dataset, height = 260, yDomain, valueSuffix }: TimeSeriesChartProps) {
  const { labels, series, threshold } = dataset;

  if (labels.length === 0 || series.length === 0) {
    return (
      <div
        className="flex items-center justify-center rounded-lg border border-dashed border-border text-sm text-muted-foreground"
        style={{ height }}
      >
        Sem leituras neste período.
      </div>
    );
  }

  // Pivota {labels, series[]} nas rows {t, [nome da série]: valor} do Recharts.
  const rows = labels.map((t, i) => ({
    t,
    ...Object.fromEntries(series.map((s) => [s.name, s.data[i]])),
  }));

  // Mostra a data no eixo X apenas quando o período cruza mais de um dia.
  const firstDay = parseLabel(labels[0])?.toDateString();
  const lastDay = parseLabel(labels[labels.length - 1])?.toDateString();
  const withDate = firstDay !== lastDay;

  // Domínio do eixo Y: em linhas o baseline não precisa ser zero — sem isso a
  // série fica achatada no topo. Com threshold, garante que a linha de limite
  // sempre caiba na escala.
  const domain: React.ComponentProps<typeof YAxis>["domain"] =
    yDomain ??
    (threshold !== undefined
      ? [0, (dataMax: number) => Math.max(Math.ceil(dataMax * 1.05), Math.ceil(threshold * 1.15))]
      : ["auto", "auto"]);

  return (
    <div>
      <div style={{ height }}>
        <ResponsiveContainer width="100%" height="100%">
          <LineChart data={rows} margin={{ top: 8, right: 12, bottom: 0, left: 0 }}>
            <CartesianGrid vertical={false} stroke="var(--border)" strokeWidth={1} />
            <XAxis
              dataKey="t"
              tickFormatter={(value: string) => formatTick(value, withDate)}
              tick={{ fill: "var(--muted-foreground)", fontSize: 12 }}
              tickLine={false}
              axisLine={{ stroke: "var(--border)" }}
              minTickGap={32}
            />
            <YAxis
              domain={domain}
              tickFormatter={(value: number) => `${value.toLocaleString("pt-BR")}${valueSuffix ?? ""}`}
              tick={{ fill: "var(--muted-foreground)", fontSize: 12 }}
              tickLine={false}
              axisLine={false}
              width={valueSuffix ? 48 : 56}
            />
            <Tooltip
              content={(props: TooltipContentProps) => (
                <ChartTooltip {...props} valueSuffix={valueSuffix} />
              )}
              cursor={{ stroke: "var(--border)", strokeWidth: 1 }}
            />
            {threshold !== undefined ? (
              <ReferenceLine
                y={threshold}
                stroke="var(--danger)"
                strokeDasharray="4 4"
                label={{
                  value: `limite ${threshold}`,
                  position: "insideTopRight",
                  fill: "var(--muted-foreground)",
                  fontSize: 12,
                }}
              />
            ) : null}
            {series.map((s, index) => (
              <Line
                key={s.name}
                type="monotone"
                dataKey={s.name}
                stroke={SERIES_COLORS[index % SERIES_COLORS.length]}
                strokeWidth={2}
                strokeLinecap="round"
                strokeLinejoin="round"
                dot={false}
                activeDot={{ r: 4, strokeWidth: 2, stroke: "var(--card)" }}
                isAnimationActive={false}
              />
            ))}
          </LineChart>
        </ResponsiveContainer>
      </div>

      {series.length >= 2 ? (
        <div className="mt-2 flex flex-wrap items-center gap-x-4 gap-y-1">
          {series.map((s, index) => (
            <span key={s.name} className="flex items-center gap-1.5 text-xs text-muted-foreground">
              <span
                aria-hidden="true"
                className="inline-block h-2.5 w-2.5 rounded-full"
                style={{ background: SERIES_COLORS[index % SERIES_COLORS.length] }}
              />
              {s.name}
            </span>
          ))}
        </div>
      ) : null}
    </div>
  );
}
