// Client da API do backend (fetch nativo, sem libs).
//
// Todas as chamadas usam paths relativos: os rewrites em next.config.ts fazem
// proxy same-origin para o backend, o que dispensa CORS e faz o cookie
// HttpOnly de refresh (Path=/auth) acompanhar as requests de /auth/*.
//
// O access token (JWT, 15 min) vive apenas em memória neste módulo; quando uma
// request autenticada recebe 401, tentamos um único POST /auth/refresh
// (single-flight) e repetimos a request original uma vez.

import type {
  AnalyticsDashboard,
  ApiEnvelope,
  AuthTokens,
  ChartMeta,
  Conexao,
  Dispositivo,
  DispositivoLink,
  HardwareInfo,
  Usuario,
} from "./types";

export class ApiError extends Error {
  readonly status: number;

  constructor(status: number, message: string) {
    super(message);
    this.name = "ApiError";
    this.status = status;
  }
}

let accessToken: string | null = null;

export function setAccessToken(token: string | null) {
  accessToken = token;
}

export function getAccessToken() {
  return accessToken;
}

/** Chamado quando o refresh falha (sessão expirada de verdade). */
let onSessionExpired: (() => void) | null = null;

export function setOnSessionExpired(handler: (() => void) | null) {
  onSessionExpired = handler;
}

async function parseEnvelope<T>(response: Response): Promise<T> {
  let envelope: ApiEnvelope<T> | null = null;
  try {
    envelope = (await response.json()) as ApiEnvelope<T>;
  } catch {
    // Corpo não-JSON (ex.: backend fora do ar atrás do proxy).
  }

  if (!envelope) {
    throw new ApiError(response.status, `Erro de comunicação com o servidor (HTTP ${response.status}).`);
  }
  if (!envelope.ok) {
    throw new ApiError(response.status, envelope.error || "Erro desconhecido.");
  }
  return envelope.data;
}

// Single-flight: N requests que tomam 401 ao mesmo tempo disparam UM refresh.
let refreshPromise: Promise<AuthTokens> | null = null;

function refreshOnce(): Promise<AuthTokens> {
  if (!refreshPromise) {
    refreshPromise = (async () => {
      const response = await fetch("/auth/refresh", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: "{}",
      });
      const tokens = await parseEnvelope<AuthTokens>(response);
      setAccessToken(tokens.access_token);
      return tokens;
    })().finally(() => {
      refreshPromise = null;
    });
  }
  return refreshPromise;
}

type RequestOptions = {
  method?: string;
  body?: unknown;
};

async function request<T>(path: string, options: RequestOptions = {}, retrying = false): Promise<T> {
  const response = await fetch(path, {
    method: options.method ?? "GET",
    headers: {
      "Content-Type": "application/json",
      ...(accessToken ? { Authorization: `Bearer ${accessToken}` } : {}),
    },
    body: options.body !== undefined ? JSON.stringify(options.body) : undefined,
  });

  // 401 fora de /auth/*: access token expirado — renova e repete uma única vez.
  if (response.status === 401 && !path.startsWith("/auth/") && !retrying) {
    try {
      await refreshOnce();
    } catch {
      setAccessToken(null);
      onSessionExpired?.();
      throw new ApiError(401, "Sessão expirada. Entre novamente.");
    }
    return request<T>(path, options, true);
  }

  return parseEnvelope<T>(response);
}

export const api = {
  auth: {
    register(nome: string, email: string, senha: string) {
      return request<Pick<Usuario, "id" | "nome" | "email">>("/auth/register", {
        method: "POST",
        body: { nome, email, senha },
      });
    },
    async login(email: string, senha: string) {
      const tokens = await request<AuthTokens>("/auth/login", {
        method: "POST",
        body: { email, senha },
      });
      setAccessToken(tokens.access_token);
      return tokens;
    },
    /** Restaura a sessão a partir do cookie HttpOnly de refresh. */
    refresh() {
      return refreshOnce();
    },
    async logout() {
      try {
        await request<null>("/auth/logout", { method: "POST", body: {} });
      } finally {
        setAccessToken(null);
      }
    },
    me() {
      return request<Pick<Usuario, "id" | "email">>("/auth/me");
    },
  },

  conexoes: {
    list() {
      return request<Conexao[]>("/conections");
    },
    get(id: number) {
      return request<Conexao>(`/conections/${id}`);
    },
    create(input: { nome: string; can_active?: boolean }) {
      return request<{ id: number }>("/conections", { method: "POST", body: input });
    },
    update(id: number, input: { nome: string; can_active?: boolean; is_active?: boolean }) {
      return request<null>(`/conections/${id}`, { method: "PUT", body: input });
    },
    remove(id: number) {
      return request<null>(`/conections/${id}`, { method: "DELETE" });
    },
  },

  dispositivos: {
    list() {
      return request<Dispositivo[]>("/dispositivos");
    },
    create(input: { nome: string; tipo: string }) {
      return request<{ id: number }>("/dispositivos", { method: "POST", body: input });
    },
    remove(id: number) {
      return request<null>(`/dispositivos/${id}`, { method: "DELETE" });
    },
    links() {
      return request<DispositivoLink[]>("/dispositivos/links");
    },
    link(id_conection: number, id_dispositivo: number) {
      return request<null>("/dispositivos/link", {
        method: "POST",
        body: { id_conection, id_dispositivo },
      });
    },
    unlink(id_conection: number, id_dispositivo: number) {
      return request<null>("/dispositivos/unlink", {
        method: "POST",
        body: { id_conection, id_dispositivo },
      });
    },
  },

  hardware: {
    info(id: number) {
      return request<HardwareInfo>(`/hardware/${id}/info`);
    },
    setConfig(id: number, input: { E_b: number; timer: number }) {
      return request<null>(`/hardware/${id}/config`, { method: "POST", body: input });
    },
  },

  analytics: {
    dashboard(id: number, limite = 50) {
      return request<AnalyticsDashboard>(`/analytics/${id}/dashboard?limite=${limite}`);
    },
    charts() {
      return request<ChartMeta[]>("/analytics/charts");
    },
  },
};
