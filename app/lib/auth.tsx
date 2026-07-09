"use client";

// Contexto de autenticação.
//
// O access token vive só em memória (lib/api.ts) e se perde no reload; a
// sessão é restaurada no mount via POST /auth/refresh (cookie HttpOnly).
// O cookie legível `sixseven_session` é apenas um MARCADOR (não contém token)
// para o proxy.ts conseguir proteger /dashboard no servidor — a proteção real
// é o 401 da API.

import {
  createContext,
  useCallback,
  useContext,
  useEffect,
  useState,
  type ReactNode,
} from "react";
import { useRouter } from "next/navigation";
import { api, setAccessToken, setOnSessionExpired } from "./api";
import type { AuthTokens, Usuario } from "./types";

const SESSION_COOKIE = "sixseven_session";

function setSessionCookie() {
  document.cookie = `${SESSION_COOKIE}=1; Path=/; Max-Age=2592000; SameSite=Lax`;
}

function clearSessionCookie() {
  document.cookie = `${SESSION_COOKIE}=; Path=/; Max-Age=0; SameSite=Lax`;
}

export type AuthStatus = "loading" | "authenticated" | "unauthenticated";

type AuthContextValue = {
  usuario: Usuario | null;
  status: AuthStatus;
  login: (email: string, senha: string) => Promise<void>;
  register: (nome: string, email: string, senha: string) => Promise<void>;
  logout: () => Promise<void>;
};

const AuthContext = createContext<AuthContextValue | null>(null);

export function AuthProvider({ children }: { children: ReactNode }) {
  const router = useRouter();
  const [usuario, setUsuario] = useState<Usuario | null>(null);
  const [status, setStatus] = useState<AuthStatus>("loading");

  const applySession = useCallback((tokens: AuthTokens) => {
    setAccessToken(tokens.access_token);
    setUsuario(tokens.usuario);
    setStatus("authenticated");
    setSessionCookie();
  }, []);

  const clearSession = useCallback(() => {
    setAccessToken(null);
    setUsuario(null);
    setStatus("unauthenticated");
    clearSessionCookie();
  }, []);

  // Restaura a sessão ao montar (reload/aba nova).
  useEffect(() => {
    let cancelled = false;

    api.auth
      .refresh()
      .then((tokens) => {
        if (!cancelled) applySession(tokens);
      })
      .catch(() => {
        if (!cancelled) clearSession();
      });

    return () => {
      cancelled = true;
    };
  }, [applySession, clearSession]);

  // Sessão expirada no meio do uso (refresh falhou em um 401): volta pro login.
  useEffect(() => {
    setOnSessionExpired(() => {
      clearSession();
      router.push("/login");
    });
    return () => setOnSessionExpired(null);
  }, [clearSession, router]);

  const login = useCallback(
    async (email: string, senha: string) => {
      const tokens = await api.auth.login(email, senha);
      applySession(tokens);
    },
    [applySession],
  );

  const register = useCallback(
    async (nome: string, email: string, senha: string) => {
      await api.auth.register(nome, email, senha);
      // Login automático após o cadastro.
      const tokens = await api.auth.login(email, senha);
      applySession(tokens);
    },
    [applySession],
  );

  const logout = useCallback(async () => {
    try {
      await api.auth.logout();
    } catch {
      // Mesmo se a API falhar, encerra a sessão local.
    }
    clearSession();
    router.push("/login");
  }, [clearSession, router]);

  return (
    <AuthContext.Provider value={{ usuario, status, login, register, logout }}>
      {children}
    </AuthContext.Provider>
  );
}

export function useAuth(): AuthContextValue {
  const context = useContext(AuthContext);
  if (!context) {
    throw new Error("useAuth deve ser usado dentro de <AuthProvider>.");
  }
  return context;
}
