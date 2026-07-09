// Tipos espelhando os modelos e respostas do backend (ver backend/routes.md).
// Os nomes de campos seguem o backend (pt-BR e notação do firmware: E_b, M_er...).

/** Envelope padrão de todas as rotas do site: {ok:true,data} | {ok:false,error}. */
export type ApiEnvelope<T> =
  | { ok: true; data: T }
  | { ok: false; error: string };

export type Usuario = {
  id: number;
  nome: string;
  email: string;
};

/** Resposta de POST /auth/login e /auth/refresh. */
export type AuthTokens = {
  access_token: string;
  token_type: "Bearer";
  expires_in: number;
  refresh_token: string;
  usuario: Usuario;
};

/** Uma conexão representa um hardware físico (ESP32) do usuário. */
export type Conexao = {
  id: number;
  nome: string;
  can_active: boolean;
  is_active: boolean;
  id_usuario: number;
};

export type Dispositivo = {
  id: number;
  nome: string;
  tipo: string;
  id_usuario: number;
};

/** Vínculo N:N — atenção: a resposta usa plural (id_conections), o body de link/unlink usa singular. */
export type DispositivoLink = {
  id_conections: number;
  id_dispositivos: number;
};

/**
 * Última leitura do hardware (GET /hardware/{id}/info).
 * E_b: energia base configurada · E_r: energia recebida · timer: intervalo de envio (ms)
 * M_er: média móvel de E_r · V_mer: variação · M_vmer: média das variações.
 * Derivados: estavel (|V_mer| <= 150) e percentual_energia (E_r/E_b*100).
 */
export type HardwareInfo = {
  id: number;
  E_b: number;
  E_r: number;
  timer: number;
  M_er: number;
  V_mer: number;
  M_vmer: number;
  estavel: boolean;
  percentual_energia: number;
};

export type ChartSeries = {
  name: string;
  data: number[];
};

export type ChartDataset = {
  labels: string[];
  series: ChartSeries[];
  /** Presente apenas no gráfico de variação (limiar de estabilidade = 150). */
  threshold?: number;
};

export type DashboardChart = {
  key: string;
  label: string;
  dataset: ChartDataset;
};

/** Quando vazio=true (sem leituras), os demais campos não vêm na resposta. */
export type AnalyticsResumo = {
  vazio: boolean;
  e_r_min?: number;
  e_r_max?: number;
  e_r_media?: number;
  pct_estavel?: number;
  ultimo_estavel?: boolean;
  ultimo_percentual?: number;
};

/** Resposta de GET /analytics/{id}/dashboard. */
export type AnalyticsDashboard = {
  id_conection: number;
  pontos: number;
  charts: DashboardChart[];
  resumo: AnalyticsResumo;
};

/** Item de GET /analytics/charts (estratégias de gráfico disponíveis). */
export type ChartMeta = {
  key: string;
  label: string;
};
