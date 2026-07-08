import { forwardRef } from "react";
import { cn } from "@/lib/cn";
import { fieldBase, fieldBorder } from "./field";

export type InputProps = React.InputHTMLAttributes<HTMLInputElement> & {
  invalid?: boolean;
};

export const Input = forwardRef<HTMLInputElement, InputProps>(function Input(
  { className, invalid, ...props },
  ref,
) {
  return (
    <input
      ref={ref}
      aria-invalid={invalid || undefined}
      className={cn(fieldBase, "h-10", fieldBorder(invalid), className)}
      {...props}
    />
  );
});
