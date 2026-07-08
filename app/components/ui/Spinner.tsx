import { cn } from "@/lib/cn";

export type SpinnerProps = React.SVGProps<SVGSVGElement>;

/** Indicador de carregamento. O tamanho é controlado por width/height (padrão 20). */
export function Spinner({ className, width = 20, height = 20, ...props }: SpinnerProps) {
  return (
    <svg
      width={width}
      height={height}
      viewBox="0 0 24 24"
      fill="none"
      role="status"
      aria-label="Carregando"
      className={cn("animate-spin", className)}
      {...props}
    >
      <circle
        className="opacity-25"
        cx="12"
        cy="12"
        r="10"
        stroke="currentColor"
        strokeWidth="4"
      />
      <path
        className="opacity-75"
        fill="currentColor"
        d="M4 12a8 8 0 0 1 8-8v4a4 4 0 0 0-4 4H4z"
      />
    </svg>
  );
}
