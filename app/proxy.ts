import { NextResponse } from "next/server";

/**
 * Proxy — no Next.js 16 o antigo `middleware` foi renomeado para `proxy`
 * (roda no runtime Node.js, não configurável). Por enquanto é apenas um
 * pass-through: a autenticação ainda não existe.
 *
 * Quando a auth for implementada, validar a sessão aqui e redirecionar os
 * acessos não autenticados a /dashboard para /login. Ex.:
 *   const session = request.cookies.get("session");
 *   if (!session) return NextResponse.redirect(new URL("/login", request.url));
 */
export function proxy() {
  // TODO: validar sessão (cookie/token) quando a API de auth existir.
  return NextResponse.next();
}

export const config = {
  matcher: ["/dashboard/:path*"],
};
