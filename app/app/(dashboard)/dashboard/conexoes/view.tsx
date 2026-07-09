"use client";

// CRUD de conexões (hardwares do usuário). O form no topo alterna entre
// criar e editar; a exclusão usa confirmação em duas etapas no próprio row.

import { useCallback, useEffect, useState } from "react";
import {
  Button,
  Card,
  CardContent,
  CardDescription,
  CardHeader,
  CardTitle,
  Checkbox,
  FormField,
  Input,
  Spinner,
} from "@/components/ui";
import { api, ApiError } from "@/lib/api";
import { useAuth } from "@/lib/auth";
import type { Conexao } from "@/lib/types";

function Badge({ children, active }: { children: React.ReactNode; active: boolean }) {
  return (
    <span
      className={
        active
          ? "rounded-full bg-primary/10 px-2 py-0.5 text-xs font-medium text-primary"
          : "rounded-full bg-muted px-2 py-0.5 text-xs font-medium text-muted-foreground"
      }
    >
      {children}
    </span>
  );
}

export function ConexoesView() {
  const { status } = useAuth();
  const [conexoes, setConexoes] = useState<Conexao[] | null>(null);
  const [listError, setListError] = useState<string | null>(null);

  // Form (criar ou editar).
  const [editing, setEditing] = useState<Conexao | null>(null);
  const [nome, setNome] = useState("");
  const [canActive, setCanActive] = useState(false);
  const [isActive, setIsActive] = useState(false);
  const [nomeError, setNomeError] = useState<string | null>(null);
  const [formError, setFormError] = useState<string | null>(null);
  const [saving, setSaving] = useState(false);

  // Exclusão em duas etapas: id aguardando confirmação.
  const [confirmingId, setConfirmingId] = useState<number | null>(null);
  const [removingId, setRemovingId] = useState<number | null>(null);

  const reload = useCallback(() => {
    return api.conexoes
      .list()
      .then((list) => {
        setConexoes(list);
        setListError(null);
      })
      .catch((err: unknown) => {
        setListError(err instanceof ApiError ? err.message : "Não foi possível carregar as conexões.");
      });
  }, []);

  useEffect(() => {
    if (status !== "authenticated") return;
    void reload();
  }, [status, reload]);

  function startEdit(conexao: Conexao) {
    setEditing(conexao);
    setNome(conexao.nome);
    setCanActive(conexao.can_active);
    setIsActive(conexao.is_active);
    setNomeError(null);
    setFormError(null);
    setConfirmingId(null);
  }

  function resetForm() {
    setEditing(null);
    setNome("");
    setCanActive(false);
    setIsActive(false);
    setNomeError(null);
    setFormError(null);
  }

  async function handleSubmit(event: React.FormEvent<HTMLFormElement>) {
    event.preventDefault();
    if (!nome.trim()) {
      setNomeError("Informe o nome da conexão.");
      return;
    }
    setNomeError(null);
    setFormError(null);
    setSaving(true);
    try {
      if (editing) {
        await api.conexoes.update(editing.id, {
          nome: nome.trim(),
          can_active: canActive,
          is_active: isActive,
        });
      } else {
        await api.conexoes.create({ nome: nome.trim(), can_active: canActive });
      }
      resetForm();
      await reload();
    } catch (err) {
      setFormError(err instanceof ApiError ? err.message : "Não foi possível salvar a conexão.");
    } finally {
      setSaving(false);
    }
  }

  async function handleRemove(id: number) {
    setRemovingId(id);
    try {
      await api.conexoes.remove(id);
      if (editing?.id === id) resetForm();
      await reload();
    } catch (err) {
      setListError(err instanceof ApiError ? err.message : "Não foi possível excluir a conexão.");
    } finally {
      setRemovingId(null);
      setConfirmingId(null);
    }
  }

  return (
    <div className="mx-auto flex max-w-4xl flex-col gap-6">
      <div>
        <h1 className="text-2xl font-semibold tracking-tight text-foreground">Conexões</h1>
        <p className="text-sm text-muted-foreground">
          Cada conexão representa um hardware (ESP32) que envia leituras de energia.
        </p>
      </div>

      <Card>
        <CardHeader>
          <CardTitle>{editing ? `Editar "${editing.nome}"` : "Nova conexão"}</CardTitle>
          <CardDescription>
            {editing
              ? "Altere os dados e salve, ou cancele a edição."
              : "Cadastre um hardware para começar a receber leituras."}
          </CardDescription>
        </CardHeader>
        <CardContent>
          <form noValidate onSubmit={handleSubmit} className="flex flex-col gap-4">
            <FormField label="Nome" htmlFor="conexao-nome" required error={nomeError}>
              <Input
                id="conexao-nome"
                placeholder="Ex.: Hardware Sala"
                value={nome}
                invalid={!!nomeError}
                onChange={(event) => setNome(event.target.value)}
              />
            </FormField>

            <div className="flex flex-wrap gap-4">
              <Checkbox
                id="conexao-can-active"
                label="Pode ser ativada"
                checked={canActive}
                onChange={(event) => setCanActive(event.target.checked)}
              />
              {editing ? (
                <Checkbox
                  id="conexao-is-active"
                  label="Ativa"
                  checked={isActive}
                  onChange={(event) => setIsActive(event.target.checked)}
                />
              ) : null}
            </div>

            {formError ? <p className="text-sm text-danger">{formError}</p> : null}

            <div className="flex gap-2">
              <Button type="submit" loading={saving}>
                {editing ? "Salvar alterações" : "Criar conexão"}
              </Button>
              {editing ? (
                <Button type="button" variant="ghost" onClick={resetForm}>
                  Cancelar
                </Button>
              ) : null}
            </div>
          </form>
        </CardContent>
      </Card>

      <Card>
        <CardHeader>
          <CardTitle>Minhas conexões</CardTitle>
        </CardHeader>
        <CardContent>
          {listError ? <p className="mb-3 text-sm text-danger">{listError}</p> : null}

          {conexoes === null ? (
            <div className="flex h-24 items-center justify-center">
              <Spinner />
            </div>
          ) : conexoes.length === 0 ? (
            <p className="text-sm text-muted-foreground">Nenhuma conexão cadastrada ainda.</p>
          ) : (
            <ul>
              {conexoes.map((conexao) => (
                <li
                  key={conexao.id}
                  className="flex flex-wrap items-center gap-3 border-b border-border py-3 last:border-b-0"
                >
                  <div className="flex min-w-0 flex-1 flex-col gap-1">
                    <span className="truncate text-sm font-medium text-foreground">{conexao.nome}</span>
                    <span className="flex gap-2">
                      <Badge active={conexao.is_active}>{conexao.is_active ? "Ativa" : "Inativa"}</Badge>
                      <Badge active={conexao.can_active}>
                        {conexao.can_active ? "Pode ativar" : "Ativação bloqueada"}
                      </Badge>
                    </span>
                  </div>

                  <div className="flex items-center gap-2">
                    <Button variant="outline" size="sm" onClick={() => startEdit(conexao)}>
                      Editar
                    </Button>
                    {confirmingId === conexao.id ? (
                      <>
                        <Button
                          variant="danger"
                          size="sm"
                          loading={removingId === conexao.id}
                          onClick={() => void handleRemove(conexao.id)}
                        >
                          Confirmar exclusão
                        </Button>
                        <Button variant="ghost" size="sm" onClick={() => setConfirmingId(null)}>
                          Cancelar
                        </Button>
                      </>
                    ) : (
                      <Button variant="ghost" size="sm" onClick={() => setConfirmingId(conexao.id)}>
                        Excluir
                      </Button>
                    )}
                  </div>
                </li>
              ))}
            </ul>
          )}
        </CardContent>
      </Card>
    </div>
  );
}
