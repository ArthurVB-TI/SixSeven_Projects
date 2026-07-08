export type ClassValue = string | number | null | false | undefined;

/**
 * Junta classes condicionais em uma única string, ignorando valores falsy.
 * Helper mínimo sem dependências (clsx/tailwind-merge). As variantes dos
 * componentes são compostas de forma a não gerar classes Tailwind conflitantes.
 *
 * @example cn("px-4", isActive && "bg-primary", className)
 */
export function cn(...classes: ClassValue[]): string {
  return classes.filter(Boolean).join(" ");
}
