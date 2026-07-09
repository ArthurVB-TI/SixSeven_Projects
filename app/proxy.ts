import { NextResponse, type NextRequest } from "next/server";

/**
 * Proxy — no Next.js 16 o antigo `middleware` foi renomeado para `proxy`.
 *
 * Protege /dashboard checando o cookie marcador `sixseven_session`, gravado
 * pelo AuthProvider no login. O cookie é só um sinalizador (não contém token —
 * o access token vive em memória e o refresh token é HttpOnly com Path=/auth,
 * invisível aqui); a proteção real continua sendo o 401 da API.
 */
export function proxy(request: NextRequest) {
  const hasSession = request.cookies.has("sixseven_session");
  const { pathname } = request.nextUrl;

  if (pathname.startsWith("/dashboard") && !hasSession) {
    return NextResponse.redirect(new URL("/login", request.url));
  }

  if ((pathname === "/login" || pathname === "/register") && hasSession) {
    return NextResponse.redirect(new URL("/dashboard", request.url));
  }

  return NextResponse.next();
}

export const config = {
  matcher: ["/dashboard/:path*", "/login", "/register"],
};
