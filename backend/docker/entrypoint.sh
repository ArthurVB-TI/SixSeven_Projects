#!/usr/bin/env bash
# =============================================================
#  entrypoint.sh — sobe o backend dentro do container.
#
#  1) Gera config/app.config.json a partir das variáveis de
#     ambiente (o binário só sabe ler o JSON — segredos nunca
#     ficam na imagem).
#  2) Espera o MySQL aceitar conexões.
#  3) (opcional, DB_AUTO_INIT=true) Se o schema ainda não existe,
#     roda os SQLs de DB/ na ordem correta — inclui data.sql
#     (seed de demonstração) quando DB_SEED=true.
#  4) Executa o sixseven_backend.
#
#  Variáveis (com defaults):
#   PORT=3000                    porta HTTP do backend
#   APP_ENV=production
#   NUMBER_OF_THREADS=0          0 = nº de núcleos da máquina
#   DB_HOST / DB_PORT=3306 / DB_USER=root
#   DB_PASSWORD                  (obrigatória)
#   DB_NAME=SixSeven_Projects    (os SQLs de DB/ usam esse nome fixo)
#   DB_CONNECTIONS=4             pool do Drogon
#   JWT_SECRET                   (obrigatória — ex.: openssl rand -hex 32)
#   ACCESS_TOKEN_TTL=900         segundos
#   REFRESH_TOKEN_TTL=2592000    segundos (30 dias)
#   BCRYPT_COST=10
#   CORS_ALLOW_ORIGIN=*          origem do front (com rewrites do Next
#                                o browser nem cruza origem, mas fica
#                                configurável)
#   LOG_LEVEL=INFO
#   DB_AUTO_INIT=true            cria o schema na 1ª subida
#   DB_SEED=true                 inclui data.sql (usuário de teste)
#   DB_WAIT_TIMEOUT=90           segundos esperando o MySQL
# =============================================================
set -e

: "${PORT:=3000}"
: "${APP_ENV:=production}"
: "${NUMBER_OF_THREADS:=0}"
: "${DB_HOST:=127.0.0.1}"
: "${DB_PORT:=3306}"
: "${DB_NAME:=SixSeven_Projects}"
: "${DB_USER:=root}"
: "${DB_PASSWORD:?Defina DB_PASSWORD}"
: "${DB_CONNECTIONS:=4}"
: "${JWT_SECRET:?Defina JWT_SECRET (ex.: openssl rand -hex 32)}"
: "${ACCESS_TOKEN_TTL:=900}"
: "${REFRESH_TOKEN_TTL:=2592000}"
: "${BCRYPT_COST:=10}"
: "${CORS_ALLOW_ORIGIN:=*}"
: "${LOG_LEVEL:=INFO}"
: "${DB_AUTO_INIT:=true}"
: "${DB_SEED:=true}"
: "${DB_WAIT_TIMEOUT:=90}"

if [ "$DB_NAME" != "SixSeven_Projects" ]; then
    echo "AVISO: os scripts de DB/ criam o database 'SixSeven_Projects'." >&2
    echo "       Com DB_NAME=$DB_NAME o auto-init não funciona — ajuste os SQLs." >&2
fi

# ---- 1) config/app.config.json ----
mkdir -p config storage/logs storage/uploads

cat > config/app.config.json <<EOF
{
  "app_name": "SixSeven_Projects",
  "app_env": "${APP_ENV}",
  "listeners": [
    { "address": "0.0.0.0", "port": ${PORT}, "https": false }
  ],
  "app": {
    "number_of_threads": ${NUMBER_OF_THREADS},
    "document_root": "./public",
    "upload_path": "./storage/uploads",
    "log": {
      "log_path": "./storage/logs",
      "logfile_base_name": "sixseven",
      "log_level": "${LOG_LEVEL}"
    },
    "enable_session": false,
    "max_connections": 100000
  },
  "db_clients": [
    {
      "name": "default",
      "rdbms": "mysql",
      "host": "${DB_HOST}",
      "port": ${DB_PORT},
      "dbname": "${DB_NAME}",
      "user": "${DB_USER}",
      "passwd": "${DB_PASSWORD}",
      "is_fast": false,
      "connection_number": ${DB_CONNECTIONS},
      "character_set": "utf8mb4"
    }
  ],
  "custom_config": {
    "app_env": "${APP_ENV}",
    "security": {
      "jwt_secret": "${JWT_SECRET}",
      "access_token_ttl_seconds": ${ACCESS_TOKEN_TTL},
      "refresh_token_ttl_seconds": ${REFRESH_TOKEN_TTL},
      "bcrypt_cost": ${BCRYPT_COST},
      "cors_allow_origin": "${CORS_ALLOW_ORIGIN}"
    }
  }
}
EOF

mysql_cmd() {
    mysql --connect-timeout=3 -h"$DB_HOST" -P"$DB_PORT" \
          -u"$DB_USER" -p"$DB_PASSWORD" "$@"
}

# ---- 2) espera o MySQL ----
echo "Esperando MySQL em ${DB_HOST}:${DB_PORT}..."
elapsed=0
until mysql_cmd -e "SELECT 1" >/dev/null 2>&1; do
    elapsed=$((elapsed + 3))
    if [ "$elapsed" -ge "$DB_WAIT_TIMEOUT" ]; then
        echo "ERRO: MySQL não respondeu em ${DB_WAIT_TIMEOUT}s." >&2
        exit 1
    fi
    sleep 3
done
echo "MySQL no ar."

# ---- 3) auto-init do schema (primeira subida) ----
if [ "$DB_AUTO_INIT" = "true" ]; then
    if mysql_cmd -e "SELECT 1 FROM \`${DB_NAME}\`.usuario LIMIT 1" >/dev/null 2>&1; then
        echo "Schema já existe — pulando init."
    else
        echo "Schema não encontrado — rodando SQLs de DB/..."
        files="config tables views functions CRUD logic triggers"
        [ "$DB_SEED" = "true" ] && files="$files data"
        for f in $files; do
            echo "  >> DB/$f.sql"
            if ! mysql_cmd < "DB/$f.sql"; then
                # Sem rollback aqui o schema ficaria pela metade e o
                # próximo restart pularia o init achando que está pronto.
                echo "ERRO em DB/$f.sql — desfazendo init parcial (DROP DATABASE ${DB_NAME})." >&2
                echo "Dica: o init precisa de um usuário com CREATE DATABASE/FUNCTION (use root)." >&2
                mysql_cmd -e "DROP DATABASE IF EXISTS \`${DB_NAME}\`" >/dev/null 2>&1 || true
                exit 1
            fi
        done
        echo "Banco inicializado."
    fi
fi

# ---- 4) backend ----
echo "Subindo sixseven_backend na porta ${PORT} (env: ${APP_ENV})..."
exec ./sixseven_backend
