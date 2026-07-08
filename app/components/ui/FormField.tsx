import { cn } from "@/lib/cn";
import { Label } from "./Label";

export type FormFieldProps = {
  /** Texto do label. */
  label?: React.ReactNode;
  /** id do controle associado (htmlFor do label). */
  htmlFor?: string;
  /** Texto de ajuda exibido abaixo do controle quando não há erro. */
  hint?: React.ReactNode;
  /** Mensagem de erro; quando presente, substitui o hint e ativa o estilo de erro. */
  error?: React.ReactNode;
  required?: boolean;
  className?: string;
  children: React.ReactNode;
};

/**
 * Agrupa label + controle + mensagem de ajuda/erro com espaçamento padronizado.
 *
 * @example
 * <FormField label="Email" htmlFor="email" error={errors.email}>
 *   <Input id="email" type="email" invalid={!!errors.email} />
 * </FormField>
 */
export function FormField({
  label,
  htmlFor,
  hint,
  error,
  required,
  className,
  children,
}: FormFieldProps) {
  return (
    <div className={cn("flex flex-col gap-1.5", className)}>
      {label ? (
        <Label htmlFor={htmlFor}>
          {label}
          {required ? <span className="ml-0.5 text-danger">*</span> : null}
        </Label>
      ) : null}
      {children}
      {error ? (
        <p className="text-sm text-danger">{error}</p>
      ) : hint ? (
        <p className="text-sm text-muted-foreground">{hint}</p>
      ) : null}
    </div>
  );
}
