/** Estilo base compartilhado por Input, Select e Textarea. */
export const fieldBase =
  "w-full rounded-lg border bg-card px-3 py-2 text-sm text-foreground shadow-sm transition-colors placeholder:text-muted-foreground focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-ring focus-visible:ring-offset-1 focus-visible:ring-offset-background disabled:cursor-not-allowed disabled:opacity-50";

/** Classe de borda/realce conforme o estado de validação. */
export function fieldBorder(invalid?: boolean): string {
  return invalid ? "border-danger focus-visible:ring-danger" : "border-input";
}
