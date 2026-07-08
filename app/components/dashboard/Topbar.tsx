"use client";

import { useRouter } from "next/navigation";
import { Button } from "@/components/ui";
import { LogoutIcon, MenuIcon } from "@/components/icons";

type TopbarProps = {
  onMenu: () => void;
};

export function Topbar({ onMenu }: TopbarProps) {
  const router = useRouter();

  function handleLogout() {
    // TODO: encerrar a sessão na API antes de redirecionar.
    router.push("/login");
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
          <p className="text-sm font-medium leading-tight text-foreground">Usuária Demo</p>
          <p className="text-xs leading-tight text-muted-foreground">demo@sixseven.app</p>
        </div>
        <div
          aria-hidden="true"
          className="flex h-9 w-9 items-center justify-center rounded-full bg-primary text-sm font-semibold text-primary-foreground"
        >
          UD
        </div>
        <Button variant="ghost" size="sm" onClick={handleLogout}>
          <LogoutIcon width={18} height={18} />
          <span className="hidden sm:inline">Sair</span>
        </Button>
      </div>
    </header>
  );
}
