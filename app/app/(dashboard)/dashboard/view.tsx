"use client";

// Visão geral: telemetria do hardware selecionado — KPIs, gráficos do
// analytics e configuração de E_b/timer, com polling opcional.

import { useCallback, useEffect, useRef, useState } from "react";
import Link from "next/link";
import {
  Button,
  Card,
  CardContent,
  CardHeader,
  CardTitle,
  Checkbox,
  Select,
  Spinner,
} from "@/components/ui";
import { ConnectionPicker } from "@/components/dashboard/ConnectionPicker";
import { HardwareConfigCard } from "@/components/dashboard/HardwareConfigCard";
import { KpiCards } from "@/components/dashboard/KpiCards";
import { TimeSeriesChart } from "@/components/dashboard/TimeSeriesChart";
import { api, ApiError } from "@/lib/api";
import { useAuth } from "@/lib/auth";
import type { AnalyticsDashboard, Conexao, HardwareInfo } from "@/lib/types";

const CONEXAO_STORAGE_KEY = "sixseven.conexao";
const LIMITES = [50, 100, 200, 500];

type DashboardData = {
  dashboard: AnalyticsDashboard;
  info: HardwareInfo | null;
};

type LoadState = {
  /** Par conexão:limite a que este resultado pertence. */
  key: string;
  data: DashboardData | null;
  error: string | null;
};

function useDashboardData(conexaoId: number | null, limite: number) {
  const [state, setState] = useState<LoadState | null>(null);
  const requestSeq = useRef(0);

  const reload = useCallback(() => {
    if (conexaoId === null) return Promise.resolve();
    const key = `${conexaoId}:${limite}`;
    const seq = ++requestSeq.current;
    return Promise.all([
      api.analytics.dashboard(conexaoId, limite),
      // Conexão sem nenhuma leitura ainda não tem linha em `info` (404).
      api.hardware.info(conexaoId).catch((err: unknown) => {
        if (err instanceof ApiError && err.status === 404) return null;
        throw err;
      }),
    ])
      .then(([dashboard, info]) => {
        if (seq !== requestSeq.current) return; // resposta obsoleta
        setState({ key, data: { dashboard, info }, error: null });
      })
      .catch((err: unknown) => {
        if (seq !== requestSeq.current) return;
        const message = err instanceof ApiError ? err.message : "Não foi possível carregar os dados.";
        // Falha durante um refresh mantém os dados já exibidos.
        setState((prev) => ({
          key,
          data: prev && prev.key === key ? prev.data : null,
          error: message,
        }));
      });
  }, [conexaoId, limite]);

  useEffect(() => {
    void reload();
  }, [reload]);

  // Resultado de outra conexão/limite conta como "carregando".
  const key = conexaoId === null ? null : `${conexaoId}:${limite}`;
  const current = state && state.key === key ? state : null;

  return {
    data: current?.data ?? null,
    error: current?.error ?? null,
    loading: conexaoId !== null && current === null,
    reload,
  };
}

export function DashboardView() {
  const { status } = useAuth();
  const [conexoes, setConexoes] = useState<Conexao[] | null>(null);
  const [conexoesError, setConexoesError] = useState<string | null>(null);
  const [conexaoId, setConexaoId] = useState<number | null>(null);
  const [limite, setLimite] = useState(50);
  const [autoRefresh, setAutoRefresh] = useState(false);

  const { data, loading, error, reload } = useDashboardData(conexaoId, limite);

  // Carrega as conexões assim que a sessão é confirmada.
  useEffect(() => {
    if (status !== "authenticated") return;
    let cancelled = false;
    api.conexoes
      .list()
      .then((list) => {
        if (cancelled) return;
        setConexoes(list);
        const stored = Number(localStorage.getItem(CONEXAO_STORAGE_KEY));
        const initial = list.some((c) => c.id === stored) ? stored : (list[0]?.id ?? null);
        setConexaoId(initial);
      })
      .catch((err: unknown) => {
        if (cancelled) return;
        setConexoesError(err instanceof ApiError ? err.message : "Não foi possível carregar as conexões.");
      });
    return () => {
      cancelled = true;
    };
  }, [status]);

  // Polling alinhado ao intervalo de envio do sensor (mínimo 5s).
  useEffect(() => {
    if (!autoRefresh || conexaoId === null) return;
    const intervalMs = Math.max(data?.info?.timer ?? 10000, 5000);
    const id = setInterval(() => {
      void reload();
    }, intervalMs);
    return () => clearInterval(id);
  }, [autoRefresh, conexaoId, data?.info?.timer, reload]);

  function selectConexao(id: number) {
    setConexaoId(id);
    localStorage.setItem(CONEXAO_STORAGE_KEY, String(id));
  }

  const header = (
    <div>
      <h1 className="text-2xl font-semibold tracking-tight text-foreground">Visão geral</h1>
      <p className="text-sm text-muted-foreground">Telemetria de energia do hardware selecionado.</p>
    </div>
  );

  // Sessão sendo restaurada ou lista de conexões carregando.
  if (status === "loading" || (status === "authenticated" && conexoes === null && !conexoesError)) {
    return (
      <div className="mx-auto flex max-w-6xl flex-col gap-6">
        {header}
        <div className="flex h-48 items-center justify-center">
          <Spinner />
        </div>
      </div>
    );
  }

  if (conexoesError) {
    return (
      <div className="mx-auto flex max-w-6xl flex-col gap-6">
        {header}
        <Card className="p-6">
          <p className="text-sm text-danger">{conexoesError}</p>
          <Button variant="outline" className="mt-4" onClick={() => window.location.reload()}>
            Tentar de novo
          </Button>
        </Card>
      </div>
    );
  }

  if (conexoes !== null && conexoes.length === 0) {
    return (
      <div className="mx-auto flex max-w-6xl flex-col gap-6">
        {header}
        <Card className="p-6">
          <p className="text-sm text-muted-foreground">
            Você ainda não tem nenhuma conexão cadastrada. Crie uma para começar a receber dados do
            seu hardware.
          </p>
          <Link href="/dashboard/conexoes" className="mt-4 inline-block">
            <Button>Criar conexão</Button>
          </Link>
        </Card>
      </div>
    );
  }

  const charts = data?.dashboard.charts ?? [];
  const resumo = data?.dashboard.resumo ?? null;

  return (
    <div className="mx-auto flex max-w-6xl flex-col gap-6">
      {header}

      <div className="flex flex-wrap items-center gap-3">
        <div className="w-full sm:w-64">
          <ConnectionPicker conexoes={conexoes ?? []} value={conexaoId} onChange={selectConexao} />
        </div>
        <div className="w-36">
          <Select
            aria-label="Quantidade de leituras"
            value={limite}
            onChange={(event) => setLimite(Number(event.target.value))}
          >
            {LIMITES.map((value) => (
              <option key={value} value={value}>
                {value} leituras
              </option>
            ))}
          </Select>
        </div>
        <Checkbox
          id="auto-refresh"
          label="Atualizar automaticamente"
          checked={autoRefresh}
          onChange={(event) => setAutoRefresh(event.target.checked)}
        />
        <Button variant="outline" size="sm" onClick={() => void reload()}>
          Atualizar
        </Button>
      </div>

      {error && !data ? (
        <Card className="p-6">
          <p className="text-sm text-danger">{error}</p>
          <Button variant="outline" className="mt-4" onClick={() => void reload()}>
            Tentar de novo
          </Button>
        </Card>
      ) : loading || !data ? (
        <div className="flex h-48 items-center justify-center">
          <Spinner />
        </div>
      ) : (
        <>
          {error ? (
            <p className="text-sm text-danger">Falha ao atualizar: {error}</p>
          ) : null}

          <KpiCards info={data.info} resumo={resumo} />

          <div className="flex flex-col gap-4">
            {charts.map((chart) => (
              <Card key={chart.key}>
                <CardHeader>
                  <CardTitle>{chart.label}</CardTitle>
                </CardHeader>
                <CardContent>
                  <TimeSeriesChart
                    dataset={chart.dataset}
                    valueSuffix={chart.key === "percentual" ? "%" : undefined}
                    yDomain={chart.key === "percentual" ? [0, "auto"] : undefined}
                  />
                </CardContent>
              </Card>
            ))}
          </div>

          {conexaoId !== null ? (
            <HardwareConfigCard conexaoId={conexaoId} info={data.info} onSaved={() => void reload()} />
          ) : null}
        </>
      )}
    </div>
  );
}
