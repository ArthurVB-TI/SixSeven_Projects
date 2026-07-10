# ============================================================
#  Dockerfile — backend SixSeven_Projects (Drogon + MySQL)
#
#  Multi-stage:
#   1) build   — compila o Drogon (da fonte, como no install.sh)
#                e depois o backend.
#   2) runtime — imagem enxuta só com as libs de runtime, o
#                binário, public/, DB/ (para o auto-init) e o
#                entrypoint que gera config/app.config.json a
#                partir de variáveis de ambiente.
#
#  Variáveis de ambiente do container: ver docker/entrypoint.sh
# ============================================================

# ---------- 1) build ----------
FROM ubuntu:24.04 AS build
ENV DEBIAN_FRONTEND=noninteractive

# Atenção: o suporte a MySQL do Drogon usa o cliente MariaDB
# (libmariadb-dev) — é ele que tem a API assíncrona que o ORM
# precisa. Com libmysqlclient o cmake do Drogon NÃO acha o driver
# e compila silenciosamente sem banco.
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential cmake git ca-certificates \
        libjsoncpp-dev uuid-dev zlib1g-dev libssl-dev \
        libbrotli-dev libmariadb-dev \
    && rm -rf /var/lib/apt/lists/*

# Drogon compilado da fonte (mesma receita do install.sh, com tag
# fixada para build reprodutível). Só MySQL — Postgres/SQLite fora.
ARG DROGON_VERSION=v1.9.13
RUN git clone --depth=1 --branch ${DROGON_VERSION} --recurse-submodules \
        https://github.com/drogonframework/drogon /tmp/drogon \
    && cmake -S /tmp/drogon -B /tmp/drogon/build \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_MYSQL=ON -DBUILD_POSTGRESQL=OFF -DBUILD_SQLITE=OFF \
        -DBUILD_EXAMPLES=OFF -DBUILD_CTL=OFF \
    && cmake --build /tmp/drogon/build -j"$(nproc)" \
    && cmake --install /tmp/drogon/build \
    && ldconfig \
    && rm -rf /tmp/drogon

WORKDIR /src
COPY . .
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build -j"$(nproc)"

# ---------- 2) runtime ----------
FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive

# Libs de runtime + cliente mysql (usado pelo entrypoint para
# esperar o banco e rodar os SQLs de DB/ na primeira subida).
RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates \
        libjsoncpp25 libuuid1 zlib1g libssl3t64 libbrotli1 \
        libmariadb3 mysql-client-core-8.0 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=build /src/build/sixseven_backend ./sixseven_backend
COPY public ./public
COPY DB ./DB
COPY docker/entrypoint.sh /entrypoint.sh

RUN chmod +x /entrypoint.sh ./sixseven_backend \
    && mkdir -p storage/logs storage/uploads config \
    && useradd --system --home /app sixseven \
    && chown -R sixseven:sixseven /app

USER sixseven

EXPOSE 3000
ENTRYPOINT ["/entrypoint.sh"]
