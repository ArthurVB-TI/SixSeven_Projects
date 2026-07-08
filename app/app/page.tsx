import { redirect } from "next/navigation";

export default function Home() {
  // App é orientado ao painel logado: a home redireciona para o login.
  redirect("/login");
}
