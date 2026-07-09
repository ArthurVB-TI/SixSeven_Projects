"use client";

// CRUD de dispositivos + vínculos N:N com conexões.
// Atenção à assimetria do backend: a resposta de /dispositivos/links usa
// plural (id_conections/id_dispositivos); o body de link/unlink usa singular.

import { useCallback, useEffect, useState } from "react";
import {
  Button,
  Card,
  CardContent,
  CardDescription,
  CardHeader,
  CardTitle,
  FormField,
  Input,
  Select,
  Spinner,
} from "@/components/ui";
import { CloseIcon } from "@/components/icons";
import { api, ApiError } from "@/lib/api";
import { useAuth } from "@/lib/auth";
import type { Conexao, Dispositivo, DispositivoLink } from "@/lib/types";

export function DispositivosView() {
  const { status } = useAuth();
  const [dispositivos, setDispositivos] = useState<Dispositivo[] | null>(null);
  const [conexoes, setConexoes] = useState<Conexao[]>([]);
  const [links, setLinks] = useState<DispositivoLink[]>([]);
  const [listError, setListError] = useState<string | null>(null);

  // Form de criação.
  const [nome, setNome] = useState("");
  const [tipo, setTipo] = useState("");
  const [errors, setErrors] = useState<{ nome?: string; tipo?: string }>({});
  const [formError, setFormError] = useState<string | null>(null);
  const [saving, setSaving] = useState(false);

  // Exclusão em duas etapas + operações de vínculo em andamento.
  const [confirmingId, setConfirmingId] = useState<number | null>(null);
  const [removingId, setRemovingId] = useState<number | null>(null);
  const [linkingId, setLinkingId] = useState<number | null>(null);

  const reload = useCallback(() => {
    return Promise.all([api.dispositivos.list(), api.conexoes.list(), api.dispositivos.links()])
      .then(([dispositivosList, conexoesList, linksList]) => {
        setDispositivos(dispositivosList);
        setConexoes(conexoesList);
        setLinks(linksList);
        setListError(null);
      })
      .catch((err: unknown) => {
        setListError(err instanceof ApiError ? err.message : "Não foi possível carregar os dispositivos.");
      });
  }, []);

  useEffect(() => {
    if (status !== "authenticated") return;
    void reload();
  }, [status, reload]);

  async function handleCreate(event: React.FormEvent<HTMLFormElement>) {
    event.preventDefault();
    const found: typeof errors = {};
    if (!nome.trim()) found.nome = "Informe o nome do dispositivo.";
    if (!tipo.trim()) found.tipo = "Informe o tipo (ex.: sensor, led, buzzer).";
    setErrors(found);
    if (Object.keys(found).length > 0) return;

    setFormError(null);
    setSaving(true);
    try {
      await api.dispositivos.create({ nome: nome.trim(), tipo: tipo.trim() });
      setNome("");
      setTipo("");
      await reload();
    } catch (err) {
      setFormError(err instanceof ApiError ? err.message : "Não foi possível criar o dispositivo.");
    } finally {
      setSaving(false);
    }
  }

  async function handleRemove(id: number) {
    setRemovingId(id);
    try {
      await api.dispositivos.remove(id);
      await reload();
    } catch (err) {
      setListError(err instanceof ApiError ? err.message : "Não foi possível excluir o dispositivo.");
    } finally {
      setRemovingId(null);
      setConfirmingId(null);
    }
  }

  async function handleLink(idConexao: number, idDispositivo: number) {
    setLinkingId(idDispositivo);
    try {
      await api.dispositivos.link(idConexao, idDispositivo);
      setLinks(await api.dispositivos.links());
      setListError(null);
    } catch (err) {
      setListError(err instanceof ApiError ? err.message : "Não foi possível vincular o dispositivo.");
    } finally {
      setLinkingId(null);
    }
  }

  async function handleUnlink(idConexao: number, idDispositivo: number) {
    setLinkingId(idDispositivo);
    try {
      await api.dispositivos.unlink(idConexao, idDispositivo);
      setLinks(await api.dispositivos.links());
      setListError(null);
    } catch (err) {
      setListError(err instanceof ApiError ? err.message : "Não foi possível desvincular o dispositivo.");
    } finally {
      setLinkingId(null);
    }
  }

  function conexoesDoDispositivo(dispositivoId: number): Conexao[] {
    const ids = links.filter((l) => l.id_dispositivos === dispositivoId).map((l) => l.id_conections);
    return conexoes.filter((c) => ids.includes(c.id));
  }

  function conexoesDisponiveis(dispositivoId: number): Conexao[] {
    const vinculadas = new Set(
      links.filter((l) => l.id_dispositivos === dispositivoId).map((l) => l.id_conections),
    );
    return conexoes.filter((c) => !vinculadas.has(c.id));
  }

  return (
    <div className="mx-auto flex max-w-4xl flex-col gap-6">
      <div>
        <h1 className="text-2xl font-semibold tracking-tight text-foreground">Dispositivos</h1>
        <p className="text-sm text-muted-foreground">
          Sensores e atuadores que podem ser vinculados às suas conexões.
        </p>
      </div>

      <Card>
        <CardHeader>
          <CardTitle>Novo dispositivo</CardTitle>
          <CardDescription>Cadastre um sensor ou atuador e vincule-o a uma conexão.</CardDescription>
        </CardHeader>
        <CardContent>
          <form noValidate onSubmit={handleCreate} className="flex flex-col gap-4">
            <div className="grid grid-cols-1 gap-4 sm:grid-cols-2">
              <FormField label="Nome" htmlFor="dispositivo-nome" required error={errors.nome}>
                <Input
                  id="dispositivo-nome"
                  placeholder="Ex.: Sensor Potenciômetro"
                  value={nome}
                  invalid={!!errors.nome}
                  onChange={(event) => setNome(event.target.value)}
                />
              </FormField>
              <FormField label="Tipo" htmlFor="dispositivo-tipo" required error={errors.tipo}>
                <Input
                  id="dispositivo-tipo"
                  placeholder="Ex.: sensor, led, buzzer"
                  value={tipo}
                  invalid={!!errors.tipo}
                  onChange={(event) => setTipo(event.target.value)}
                />
              </FormField>
            </div>

            {formError ? <p className="text-sm text-danger">{formError}</p> : null}

            <div>
              <Button type="submit" loading={saving}>
                Criar dispositivo
              </Button>
            </div>
          </form>
        </CardContent>
      </Card>

      <Card>
        <CardHeader>
          <CardTitle>Meus dispositivos</CardTitle>
        </CardHeader>
        <CardContent>
          {listError ? <p className="mb-3 text-sm text-danger">{listError}</p> : null}

          {dispositivos === null ? (
            <div className="flex h-24 items-center justify-center">
              <Spinner />
            </div>
          ) : dispositivos.length === 0 ? (
            <p className="text-sm text-muted-foreground">Nenhum dispositivo cadastrado ainda.</p>
          ) : (
            <ul>
              {dispositivos.map((dispositivo) => {
                const vinculadas = conexoesDoDispositivo(dispositivo.id);
                const disponiveis = conexoesDisponiveis(dispositivo.id);
                return (
                  <li
                    key={dispositivo.id}
                    className="flex flex-col gap-3 border-b border-border py-4 last:border-b-0"
                  >
                    <div className="flex flex-wrap items-center gap-3">
                      <div className="flex min-w-0 flex-1 flex-col">
                        <span className="truncate text-sm font-medium text-foreground">
                          {dispositivo.nome}
                        </span>
                        <span className="text-xs text-muted-foreground">{dispositivo.tipo}</span>
                      </div>

                      <div className="flex items-center gap-2">
                        {confirmingId === dispositivo.id ? (
                          <>
                            <Button
                              variant="danger"
                              size="sm"
                              loading={removingId === dispositivo.id}
                              onClick={() => void handleRemove(dispositivo.id)}
                            >
                              Confirmar exclusão
                            </Button>
                            <Button variant="ghost" size="sm" onClick={() => setConfirmingId(null)}>
                              Cancelar
                            </Button>
                          </>
                        ) : (
                          <Button
                            variant="ghost"
                            size="sm"
                            onClick={() => setConfirmingId(dispositivo.id)}
                          >
                            Excluir
                          </Button>
                        )}
                      </div>
                    </div>

                    <div className="flex flex-wrap items-center gap-2">
                      {vinculadas.map((conexao) => (
                        <span
                          key={conexao.id}
                          className="flex items-center gap-1 rounded-full bg-muted px-2.5 py-1 text-xs font-medium text-foreground"
                        >
                          {conexao.nome}
                          <button
                            type="button"
                            aria-label={`Desvincular de ${conexao.nome}`}
                            disabled={linkingId === dispositivo.id}
                            onClick={() => void handleUnlink(conexao.id, dispositivo.id)}
                            className="rounded-full p-0.5 text-muted-foreground hover:text-foreground disabled:opacity-50"
                          >
                            <CloseIcon width={12} height={12} />
                          </button>
                        </span>
                      ))}
                      {vinculadas.length === 0 ? (
                        <span className="text-xs text-muted-foreground">Sem vínculos.</span>
                      ) : null}

                      {disponiveis.length > 0 ? (
                        <div className="w-48">
                          <Select
                            aria-label={`Vincular ${dispositivo.nome} a uma conexão`}
                            value=""
                            disabled={linkingId === dispositivo.id}
                            onChange={(event) => {
                              const id = Number(event.target.value);
                              if (id) void handleLink(id, dispositivo.id);
                            }}
                            className="h-8 text-xs"
                          >
                            <option value="">Vincular a…</option>
                            {disponiveis.map((conexao) => (
                              <option key={conexao.id} value={conexao.id}>
                                {conexao.nome}
                              </option>
                            ))}
                          </Select>
                        </div>
                      ) : null}
                    </div>
                  </li>
                );
              })}
            </ul>
          )}
        </CardContent>
      </Card>
    </div>
  );
}
