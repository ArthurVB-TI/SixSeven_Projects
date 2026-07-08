import { forwardRef } from "react";
import { cn } from "@/lib/cn";

export type RadioProps = React.InputHTMLAttributes<HTMLInputElement> & {
  label?: React.ReactNode;
};

export const Radio = forwardRef<HTMLInputElement, RadioProps>(function Radio(
  { className, label, id, ...props },
  ref,
) {
  const input = (
    <input
      ref={ref}
      id={id}
      type="radio"
      className={cn(
        "h-4 w-4 shrink-0 border-input accent-primary focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-ring focus-visible:ring-offset-1 focus-visible:ring-offset-background disabled:opacity-50",
        className,
      )}
      {...props}
    />
  );

  if (!label) return input;

  return (
    <label htmlFor={id} className="flex select-none items-center gap-2 text-sm text-foreground">
      {input}
      <span>{label}</span>
    </label>
  );
});
