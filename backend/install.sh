#!/usr/bin/env bash
# =============================================================
#  install.sh — Instala todas as dependências do backend
#  SixSeven_Projects no Ubuntu/Debian (testado no Ubuntu 25.04)
#
#  Execute UMA VEZ, na raiz do backend:
#    chmod +x install.sh && ./install.sh
# =============================================================
set -e

BACKEND_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "==> [1/4] Instalando dependências do sistema..."
sudo apt-get update -qq
sudo apt-get install -y \
    build-essential cmake git \
    libjsoncpp-dev \
    uuid-dev \
    zlib1g-dev \
    libssl-dev \
    libpq-dev \
    libbrotli-dev \
    libmysqlclient-dev

echo "==> [2/4] Clonando e compilando o Drogon..."
DROGON_TMP=$(mktemp -d)
git clone --depth=1 https://github.com/drogonframework/drogon "$DROGON_TMP/drogon"
cd "$DROGON_TMP/drogon"
git submodule update --init --recursive
mkdir build && cd build
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_MYSQL=ON \
    -DBUILD_POSTGRESQL=ON \
    -DBUILD_SQLITE=OFF
make -j"$(nproc)"
sudo make install
sudo ldconfig
cd "$BACKEND_DIR"
rm -rf "$DROGON_TMP"

echo "==> [3/4] Baixando libbcrypt (vendorizada em composers/)..."
cd "$BACKEND_DIR/composers"
if [ ! -d libbcrypt ]; then
    git clone --depth=1 https://github.com/rg3/libbcrypt.git
fi
mkdir -p bcrypt
cp libbcrypt/bcrypt.h bcrypt/bcrypt.h
cd "$BACKEND_DIR"

echo "==> [4/4] Compilando o backend..."
mkdir -p build && cd build
cmake ..
make -j"$(nproc)"

echo ""
echo "✓ Instalação concluída!"
echo "  Binário: build/sixseven_backend"
echo "  Ajuste config/app.config.json e rode: cd build && ./sixseven_backend"
