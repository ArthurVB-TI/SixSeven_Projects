"use client";

import { Card } from "@/components/ui";
import type { AnalyticsResumo, HardwareInfo } from "@/lib/types";

type KpiCardsProps = {
  info: HardwareInfo | null;
  resumo: AnalyticsResumo | null;
};

function formatNumber(value: number | undefined): string {
  return value === undefined ? "—" : value.toLocaleString("pt-BR");
}

/** KPIs da leitura atual do hardware + resumo do período (analytics). */
export function KpiCards({ info, resumo }: KpiCardsProps) {
  const cards = [
    {
      label: "Energia recebida (E_r)",
      value: formatNumber(info?.E_r),
      hint: info ? `base E_b: ${formatNumber(info.E_b)}` : "sem leitura",
    },
    {
      label: "Percentual de energia",
      value: info ? `${info.percentual_energia}%` : "—",
      hint:
        resumo?.ultimo_percentual !== undefined
          ? `último do período: ${resumo.ultimo_percentual}%`
          : "E_r / E_b",
    },
    {
      label: "Estabilidade",
      value: null, // renderizado como badge abaixo
      hint:
        resumo?.pct_estavel !== undefined
          ? `${resumo.pct_estavel}% do período estável`
          : "variação vs limite 150",
    },
    {
      label: "E_r média (período)",
      value: formatNumber(resumo?.e_r_media),
      hint:
        resumo?.e_r_min !== undefined && resumo?.e_r_max !== undefined
          ? `mín ${formatNumber(resumo.e_r_min)} · máx ${formatNumber(resumo.e_r_max)}`
          : "sem leituras no período",
    },
  ];

  return (
    <div className="grid grid-cols-1 gap-4 sm:grid-cols-2 lg:grid-cols-4">
      {cards.map((card) => (
        <Card key={card.label} className="p-5">
          <p className="text-sm text-muted-foreground">{card.label}</p>
          {card.label === "Estabilidade" ? (
            <p className="mt-1 flex items-center gap-2 text-2xl font-semibold tracking-tight text-foreground">
              {info ? (
                <>
                  <span
                    aria-hidden="true"
                    className="inline-block h-2.5 w-2.5 rounded-full"
                    style={{ background: info.estavel ? "var(--success)" : "var(--danger)" }}
                  />
                  {info.estavel ? "Estável" : "Instável"}
                </>
              ) : (
                "—"
              )}
            </p>
          ) : (
            <p className="mt-1 text-2xl font-semibold tracking-tight text-foreground">{card.value}</p>
          )}
          <p className="mt-1 text-xs text-muted-foreground">{card.hint}</p>
        </Card>
      ))}
    </div>
  );
}
