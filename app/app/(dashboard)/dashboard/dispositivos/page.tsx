import type { Metadata } from "next";
import { DispositivosView } from "./view";

export const metadata: Metadata = {
  title: "Dispositivos",
};

export default function DispositivosPage() {
  return <DispositivosView />;
}
