export type NavIcon = "home" | "chart" | "users" | "settings";

export type NavItem = {
  label: string;
  href: string;
  icon: NavIcon;
  /** Rota ainda não implementada — renderizada como item desabilitado. */
  disabled?: boolean;
};

/** Itens de navegação da sidebar do dashboard. */
export const dashboardNav: NavItem[] = [
  { label: "Visão geral", href: "/dashboard", icon: "home" },
  { label: "Relatórios", href: "/dashboard/relatorios", icon: "chart", disabled: true },
  { label: "Usuários", href: "/dashboard/usuarios", icon: "users", disabled: true },
  { label: "Configurações", href: "/dashboard/configuracoes", icon: "settings", disabled: true },
];
