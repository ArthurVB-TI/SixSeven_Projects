#!/usr/bin/env bash
# =============================================================
#  start_local.sh — Sobe o ambiente local SEM sudo.
#
#  Este ambiente foi montado em espaço de usuário:
#   * MySQL: binário copiado para ~/.local/bin/mysqld (a cópia
#     escapa do perfil AppArmor, que só confina /usr/sbin/mysqld)
#     com datadir em ~/.local/share/sixseven-mysql/data
#     (root / root, porta 3306).
#   * Drogon: instalado em ~/.local (compilado da fonte).
#   * Backend: build/ compilado com
#       cmake .. -DCMAKE_PREFIX_PATH=$HOME/.local
#
#  Uso: ./start_local.sh
# =============================================================
set -e

MYSQL_HOME="$HOME/.local/share/sixseven-mysql"
BACKEND_DIR="$(cd "$(dirname "$0")" && pwd)"

# ---- MySQL ----
if mysqladmin --socket="$MYSQL_HOME/mysql.sock" -u root ping >/dev/null 2>&1; then
    echo "MySQL ja esta rodando."
else
    echo "Subindo MySQL local..."
    "$HOME/.local/bin/mysqld" \
        --datadir="$MYSQL_HOME/data" \
        --socket="$MYSQL_HOME/mysql.sock" \
        --pid-file="$MYSQL_HOME/mysqld.pid" \
        --log-error="$MYSQL_HOME/server.log" \
        --lc-messages-dir=/usr/share/mysql \
        --port=3306 --bind-address=127.0.0.1 --mysqlx=OFF &
    for i in $(seq 1 30); do
        mysqladmin --socket="$MYSQL_HOME/mysql.sock" -u root -proot ping >/dev/null 2>&1 && break
        sleep 1
    done
    echo "MySQL no ar (porta 3306)."
fi

# ---- Backend ----
if curl -s -m 2 http://localhost:3000/health >/dev/null 2>&1; then
    echo "Backend ja esta rodando em http://localhost:3000"
else
    echo "Subindo backend..."
    cd "$BACKEND_DIR/build"
    ./sixseven_backend &
    for i in $(seq 1 15); do
        curl -s -m 2 http://localhost:3000/health >/dev/null 2>&1 && break
        sleep 1
    done
    echo "Backend no ar: http://localhost:3000/health"
fi
