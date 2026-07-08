import { forwardRef } from "react";
import { cn } from "@/lib/cn";
import { fieldBase, fieldBorder } from "./field";

export type TextareaProps = React.TextareaHTMLAttributes<HTMLTextAreaElement> & {
  invalid?: boolean;
};

export const Textarea = forwardRef<HTMLTextAreaElement, TextareaProps>(function Textarea(
  { className, invalid, ...props },
  ref,
) {
  return (
    <textarea
      ref={ref}
      aria-invalid={invalid || undefined}
      className={cn(fieldBase, "min-h-24 resize-y", fieldBorder(invalid), className)}
      {...props}
    />
  );
});
