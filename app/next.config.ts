import type { NextConfig } from "next";

// URL do backend C++ (Drogon). Server-only: usada apenas nos rewrites abaixo —
// o browser sempre chama paths relativos (same-origin), o que dispensa CORS e
// permite que o cookie HttpOnly de refresh (Path=/auth) funcione.
const BACKEND_URL = process.env.BACKEND_URL ?? "http://localhost:3000";

const nextConfig: NextConfig = {
  async rewrites() {
    // Os paths do backend são preservados (não usar prefixo /api): o cookie de
    // refresh é emitido com Path=/auth e só acompanha requests nesse path.
    return [
      { source: "/auth/:path*", destination: `${BACKEND_URL}/auth/:path*` },
      { source: "/conections/:path*", destination: `${BACKEND_URL}/conections/:path*` },
      { source: "/dispositivos/:path*", destination: `${BACKEND_URL}/dispositivos/:path*` },
      { source: "/hardware/:path*", destination: `${BACKEND_URL}/hardware/:path*` },
      { source: "/analytics/:path*", destination: `${BACKEND_URL}/analytics/:path*` },
      { source: "/health", destination: `${BACKEND_URL}/health` },
    ];
  },
};

export default nextConfig;
