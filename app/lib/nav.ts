export type NavIcon = "home" | "chart" | "users" | "settings" | "plug" | "cpu";

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
  { label: "Conexões", href: "/dashboard/conexoes", icon: "plug" },
  { label: "Dispositivos", href: "/dashboard/dispositivos", icon: "cpu" },
];
