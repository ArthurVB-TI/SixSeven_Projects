import type { Metadata } from "next";
import { ConexoesView } from "./view";

export const metadata: Metadata = {
  title: "Conexões",
};

export default function ConexoesPage() {
  return <ConexoesView />;
}
