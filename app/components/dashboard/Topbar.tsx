"use client";

import { useState } from "react";
import { Button } from "@/components/ui";
import { LogoutIcon, MenuIcon } from "@/components/icons";
import { useAuth } from "@/lib/auth";

type TopbarProps = {
  onMenu: () => void;
};

/** Iniciais do nome para o avatar (ex.: "Arthur Vilela" → "AV"). */
function initialsOf(nome: string): string {
  const parts = nome.trim().split(/\s+/);
  const first = parts[0]?.[0] ?? "";
  const last = parts.length > 1 ? parts[parts.length - 1][0] : "";
  return (first + last).toUpperCase() || "?";
}

export function Topbar({ onMenu }: TopbarProps) {
  const { usuario, logout } = useAuth();
  const [loggingOut, setLoggingOut] = useState(false);

  async function handleLogout() {
    setLoggingOut(true);
    await logout();
  }

  return (
    <header className="sticky top-0 z-30 flex h-16 items-center gap-3 border-b border-border bg-background/80 px-4 backdrop-blur sm:px-6">
      <button
        type="button"
        onClick={onMenu}
        aria-label="Abrir menu"
        className="rounded-md p-1.5 text-muted-foreground hover:bg-muted hover:text-foreground md:hidden"
      >
        <MenuIcon width={22} height={22} />
      </button>

      <div className="flex-1" />

      <div className="flex items-center gap-3">
        <div className="hidden text-right sm:block">
          <p className="text-sm font-medium leading-tight text-foreground">
            {usuario?.nome ?? "Carregando…"}
          </p>
          <p className="text-xs leading-tight text-muted-foreground">{usuario?.email ?? ""}</p>
        </div>
        <div
          aria-hidden="true"
          className="flex h-9 w-9 items-center justify-center rounded-full bg-primary text-sm font-semibold text-primary-foreground"
        >
          {usuario ? initialsOf(usuario.nome) : "…"}
        </div>
        <Button variant="ghost" size="sm" onClick={handleLogout} loading={loggingOut}>
          <LogoutIcon width={18} height={18} />
          <span className="hidden sm:inline">Sair</span>
        </Button>
      </div>
    </header>
  );
}
