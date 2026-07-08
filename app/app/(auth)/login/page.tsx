"use client";

import { useState } from "react";
import Link from "next/link";
import { useRouter } from "next/navigation";
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

type Errors = { email?: string; password?: string };

const emailPattern = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;

export default function LoginPage() {
  const router = useRouter();
  const [email, setEmail] = useState("");
  const [password, setPassword] = useState("");
  const [remember, setRemember] = useState(false);
  const [showPassword, setShowPassword] = useState(false);
  const [errors, setErrors] = useState<Errors>({});
  const [loading, setLoading] = useState(false);

  function validate(): Errors {
    const next: Errors = {};
    if (!email.trim()) next.email = "Informe seu email.";
    else if (!emailPattern.test(email)) next.email = "Email inválido.";
    if (!password) next.password = "Informe sua senha.";
    return next;
  }

  async function handleSubmit(event: React.FormEvent<HTMLFormElement>) {
    event.preventDefault();
    const found = validate();
    setErrors(found);
    if (Object.keys(found).length > 0) return;

    setLoading(true);
    // TODO: integrar com a API de autenticação (enviar { email, password, remember }).
    await new Promise((resolve) => setTimeout(resolve, 800));
    setLoading(false);
    router.push("/dashboard");
  }

  return (
    <Card>
      <CardHeader>
        <CardTitle>Entrar na conta</CardTitle>
        <CardDescription>Bem-vindo de volta! Acesse com seu email e senha.</CardDescription>
      </CardHeader>

      <CardContent>
        <form noValidate onSubmit={handleSubmit} className="flex flex-col gap-4">
          <FormField label="Email" htmlFor="email" error={errors.email}>
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

          <FormField label="Senha" htmlFor="password" error={errors.password}>
            <div className="relative">
              <Input
                id="password"
                type={showPassword ? "text" : "password"}
                autoComplete="current-password"
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

          <div className="flex items-center justify-between">
            <Checkbox
              id="remember"
              label="Lembrar de mim"
              checked={remember}
              onChange={(event) => setRemember(event.target.checked)}
            />
            <button
              type="button"
              className="text-sm font-medium text-primary hover:underline"
              // TODO: navegar para o fluxo de recuperação de senha quando existir.
            >
              Esqueci minha senha
            </button>
          </div>

          <Button type="submit" loading={loading} fullWidth>
            Entrar
          </Button>
        </form>
      </CardContent>

      <CardFooter className="justify-center">
        <p className="text-sm text-muted-foreground">
          Não tem uma conta?{" "}
          <Link href="/register" className="font-medium text-primary hover:underline">
            Criar conta
          </Link>
        </p>
      </CardFooter>
    </Card>
  );
}
