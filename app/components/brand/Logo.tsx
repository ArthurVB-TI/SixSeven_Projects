import { cn } from "@/lib/cn";
import { APP_NAME } from "@/lib/site";

export type LogoProps = {
  className?: string;
  /** Exibe apenas o símbolo, sem o nome. */
  iconOnly?: boolean;
};

export function Logo({ className, iconOnly = false }: LogoProps) {
  return (
    <span className={cn("inline-flex items-center gap-2 font-semibold text-foreground", className)}>
      <span
        aria-hidden="true"
        className="inline-flex h-8 w-8 items-center justify-center rounded-lg bg-primary text-primary-foreground"
      >
        <svg
          width="18"
          height="18"
          viewBox="0 0 24 24"
          fill="none"
          stroke="currentColor"
          strokeWidth="2.2"
          strokeLinecap="round"
          strokeLinejoin="round"
        >
          <path d="M12 3 4 7v6c0 4 3.5 6.5 8 8 4.5-1.5 8-4 8-8V7l-8-4Z" />
        </svg>
      </span>
      {!iconOnly && <span className="tracking-tight">{APP_NAME}</span>}
    </span>
  );
}
