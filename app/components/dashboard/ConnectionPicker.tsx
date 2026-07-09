"use client";

import { Select } from "@/components/ui";
import type { Conexao } from "@/lib/types";

type ConnectionPickerProps = {
  conexoes: Conexao[];
  value: number | null;
  onChange: (id: number) => void;
};

/** Seletor do hardware (conexão) exibido no dashboard. */
export function ConnectionPicker({ conexoes, value, onChange }: ConnectionPickerProps) {
  return (
    <Select
      aria-label="Conexão"
      value={value ?? ""}
      onChange={(event) => onChange(Number(event.target.value))}
    >
      {conexoes.map((conexao) => (
        <option key={conexao.id} value={conexao.id}>
          {conexao.nome}
          {conexao.is_active ? "" : " (inativa)"}
        </option>
      ))}
    </Select>
  );
}
