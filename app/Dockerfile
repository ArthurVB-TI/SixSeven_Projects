# ============================================================
#  Dockerfile — front SixSeven_Projects (Next.js 16, standalone)
#
#  ATENÇÃO: BACKEND_URL é lido em BUILD time (os rewrites do
#  next.config.ts são serializados no build). Se a URL do backend
#  mudar, é preciso rebuildar a imagem. No EasyPanel, as variáveis
#  de ambiente do serviço ficam disponíveis também no build.
#
#  Default pensado para os dois serviços no mesmo projeto do
#  EasyPanel (DNS interno = <projeto>_<serviço>).
# ============================================================

# ---------- 1) dependências ----------
FROM node:22-alpine AS deps
WORKDIR /app
COPY package.json package-lock.json ./
RUN npm ci

# ---------- 2) build ----------
FROM node:22-alpine AS build
WORKDIR /app
COPY --from=deps /app/node_modules ./node_modules
COPY . .

ARG BACKEND_URL=http://sixseven_backend:3000
ENV BACKEND_URL=${BACKEND_URL} \
    NEXT_TELEMETRY_DISABLED=1

RUN npm run build

# ---------- 3) runtime ----------
FROM node:22-alpine
WORKDIR /app
ENV NODE_ENV=production \
    PORT=3001 \
    HOSTNAME=0.0.0.0 \
    NEXT_TELEMETRY_DISABLED=1

# Saída standalone: só o server.js + node_modules mínimos traçados
COPY --from=build --chown=node:node /app/.next/standalone ./
COPY --from=build --chown=node:node /app/.next/static ./.next/static
COPY --from=build --chown=node:node /app/public ./public

USER node
EXPOSE 3001
CMD ["node", "server.js"]
