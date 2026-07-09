"use client";

// Formulário de configuração do hardware (energia base e intervalo de envio).
// O site grava E_b/timer; o ESP32 puxa a config no próximo ciclo (GET /config).

import { useState } from "react";
import {
  Button,
  Card,
  CardContent,
  CardDescription,
  CardHeader,
  CardTitle,
  FormField,
  Input,
} from "@/components/ui";
import { api, ApiError } from "@/lib/api";
import type { HardwareInfo } from "@/lib/types";

type HardwareConfigCardProps = {
  conexaoId: number;
  info: HardwareInfo | null;
  /** Chamado após salvar, para o dashboard recarregar os dados. */
  onSaved: () => void;
};

type Errors = { eb?: string; timer?: string };

export function HardwareConfigCard({ conexaoId, info, onSaved }: HardwareConfigCardProps) {
  const [eb, setEb] = useState("");
  const [timer, setTimer] = useState("");
  const [errors, setErrors] = useState<Errors>({});
  const [formError, setFormError] = useState<string | null>(null);
  const [success, setSuccess] = useState(false);
  const [saving, setSaving] = useState(false);

  // Reinicia o form ao trocar de conexão ou quando a primeira leitura chega
  // (ajuste de estado durante o render — não usar effect, para o polling não
  // sobrescrever o que o usuário está digitando).
  const resetKey = `${conexaoId}:${info === null}`;
  const [prevResetKey, setPrevResetKey] = useState<string | null>(null);
  if (resetKey !== prevResetKey) {
    setPrevResetKey(resetKey);
    setEb(info ? String(info.E_b) : "");
    setTimer(info ? String(info.timer) : "");
    setErrors({});
    setFormError(null);
    setSuccess(false);
  }

  function validate(): Errors {
    const next: Errors = {};
    const ebValue = Number(eb);
    const timerValue = Number(timer);
    if (!eb.trim() || !Number.isInteger(ebValue) || ebValue < 0) {
      next.eb = "Informe um inteiro maior ou igual a 0.";
    }
    if (!timer.trim() || !Number.isInteger(timerValue) || timerValue <= 0) {
      next.timer = "Informe um inteiro maior que 0 (em ms).";
    }
    return next;
  }

  async function handleSubmit(event: React.FormEvent<HTMLFormElement>) {
    event.preventDefault();
    const found = validate();
    setErrors(found);
    setSuccess(false);
    setFormError(null);
    if (Object.keys(found).length > 0) return;

    setSaving(true);
    try {
      await api.hardware.setConfig(conexaoId, { E_b: Number(eb), timer: Number(timer) });
      setSuccess(true);
      onSaved();
    } catch (error) {
      setFormError(error instanceof ApiError ? error.message : "Não foi possível salvar a configuração.");
    } finally {
      setSaving(false);
    }
  }

  return (
    <Card>
      <CardHeader>
        <CardTitle>Configuração do hardware</CardTitle>
        <CardDescription>
          O sensor aplica os novos valores no próximo ciclo de envio.
        </CardDescription>
      </CardHeader>
      <CardContent>
        <form noValidate onSubmit={handleSubmit} className="flex flex-col gap-4">
          <div className="grid grid-cols-1 gap-4 sm:grid-cols-2">
            <FormField
              label="Energia base (E_b)"
              htmlFor="config-eb"
              error={errors.eb}
              hint="Referência para o percentual de energia."
            >
              <Input
                id="config-eb"
                type="number"
                min={0}
                step={1}
                value={eb}
                invalid={!!errors.eb}
                onChange={(event) => setEb(event.target.value)}
              />
            </FormField>

            <FormField
              label="Intervalo de envio (timer)"
              htmlFor="config-timer"
              error={errors.timer}
              hint="Em milissegundos (ex.: 10000 = 10s)."
            >
              <Input
                id="config-timer"
                type="number"
                min={1}
                step={1}
                value={timer}
                invalid={!!errors.timer}
                onChange={(event) => setTimer(event.target.value)}
              />
            </FormField>
          </div>

          {formError ? <p className="text-sm text-danger">{formError}</p> : null}
          {success ? (
            <p className="text-sm" style={{ color: "var(--success)" }}>
              Configuração salva.
            </p>
          ) : null}

          <div>
            <Button type="submit" loading={saving}>
              Salvar configuração
            </Button>
          </div>
        </form>
      </CardContent>
    </Card>
  );
}
