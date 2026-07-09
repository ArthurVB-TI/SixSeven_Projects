"use client";

import { useState } from "react";
import Link from "next/link";
import { useRouter } from "next/navigation";
import { useAuth } from "@/lib/auth";
import { ApiError } from "@/lib/api";
import {
  Button,
  Card,
  CardContent,
  CardDescription,
  CardFooter,
  CardHeader,
  CardTitle,
  Checkbox,
  FormField,
  Input,
} from "@/components/ui";
import { EyeIcon, EyeOffIcon } from "@/components/icons";

type Errors = {
  name?: string;
  email?: string;
  password?: string;
  confirm?: string;
  terms?: string;
};

const emailPattern = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;

export default function RegisterPage() {
  const router = useRouter();
  const auth = useAuth();
  const [name, setName] = useState("");
  const [email, setEmail] = useState("");
  const [password, setPassword] = useState("");
  const [confirm, setConfirm] = useState("");
  const [terms, setTerms] = useState(false);
  const [showPassword, setShowPassword] = useState(false);
  const [errors, setErrors] = useState<Errors>({});
  const [formError, setFormError] = useState<string | null>(null);
  const [loading, setLoading] = useState(false);

  function validate(): Errors {
    const next: Errors = {};
    if (name.trim().length < 2) next.name = "Informe seu nome completo.";
    if (!email.trim()) next.email = "Informe seu email.";
    else if (!emailPattern.test(email)) next.email = "Email inválido.";
    if (password.length < 8) next.password = "A senha deve ter ao menos 8 caracteres.";
    if (confirm !== password) next.confirm = "As senhas não coincidem.";
    if (!terms) next.terms = "É preciso aceitar os termos para continuar.";
    return next;
  }

  async function handleSubmit(event: React.FormEvent<HTMLFormElement>) {
    event.preventDefault();
    const found = validate();
    setErrors(found);
    if (Object.keys(found).length > 0) return;

    setFormError(null);
    setLoading(true);
    try {
      // Cadastra e já entra (o register do AuthProvider faz login automático).
      await auth.register(name.trim(), email, password);
      router.push("/dashboard");
    } catch (error) {
      if (error instanceof ApiError && error.status === 409) {
        setErrors({ email: "Este email já está cadastrado." });
      } else {
        setFormError(
          error instanceof ApiError ? error.message : "Não foi possível criar a conta. Tente novamente.",
        );
      }
      setLoading(false);
    }
  }

  return (
    <Card>
      <CardHeader>
        <CardTitle>Criar conta</CardTitle>
        <CardDescription>Preencha os dados abaixo para começar.</CardDescription>
      </CardHeader>

      <CardContent>
        <form noValidate onSubmit={handleSubmit} className="flex flex-col gap-4">
          <FormField label="Nome" htmlFor="name" required error={errors.name}>
            <Input
              id="name"
              type="text"
              autoComplete="name"
              placeholder="Seu nome completo"
              value={name}
              invalid={!!errors.name}
              onChange={(event) => setName(event.target.value)}
            />
          </FormField>

          <FormField label="Email" htmlFor="email" required error={errors.email}>
            <Input
              id="email"
              type="email"
              autoComplete="email"
              placeholder="voce@exemplo.com"
              value={email}
              invalid={!!errors.email}
              onChange={(event) => setEmail(event.target.value)}
            />
          </FormField>

          <FormField
            label="Senha"
            htmlFor="password"
            required
            error={errors.password}
            hint="Mínimo de 8 caracteres."
          >
            <div className="relative">
              <Input
                id="password"
                type={showPassword ? "text" : "password"}
                autoComplete="new-password"
                placeholder="••••••••"
                className="pr-10"
                value={password}
                invalid={!!errors.password}
                onChange={(event) => setPassword(event.target.value)}
              />
              <button
                type="button"
                onClick={() => setShowPassword((value) => !value)}
                aria-label={showPassword ? "Ocultar senha" : "Mostrar senha"}
                className="absolute right-2 top-1/2 -translate-y-1/2 rounded-md p-1.5 text-muted-foreground transition-colors hover:text-foreground"
              >
                {showPassword ? <EyeOffIcon width={18} height={18} /> : <EyeIcon width={18} height={18} />}
              </button>
            </div>
          </FormField>

          <FormField label="Confirmar senha" htmlFor="confirm" required error={errors.confirm}>
            <Input
              id="confirm"
              type={showPassword ? "text" : "password"}
              autoComplete="new-password"
              placeholder="••••••••"
              value={confirm}
              invalid={!!errors.confirm}
              onChange={(event) => setConfirm(event.target.value)}
            />
          </FormField>

          <FormField error={errors.terms}>
            <Checkbox
              id="terms"
              label="Li e aceito os termos de uso e a política de privacidade."
              checked={terms}
              onChange={(event) => setTerms(event.target.checked)}
            />
          </FormField>

          {formError ? <p className="text-sm text-danger">{formError}</p> : null}

          <Button type="submit" loading={loading} fullWidth>
            Criar conta
          </Button>
        </form>
      </CardContent>

      <CardFooter className="justify-center">
        <p className="text-sm text-muted-foreground">
          Já tem uma conta?{" "}
          <Link href="/login" className="font-medium text-primary hover:underline">
            Entrar
          </Link>
        </p>
      </CardFooter>
    </Card>
  );
}
