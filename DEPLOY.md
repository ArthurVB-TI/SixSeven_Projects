# Deploy no EasyPanel — SixSeven_Projects

Guia para subir o projeto numa VPS com [EasyPanel](https://easypanel.io).

## Arquitetura: por que dois serviços (três, com o banco)

São **3 serviços** dentro de **um projeto** do EasyPanel:

```
                      ┌─────────────────────── VPS / EasyPanel ───────────────────────┐
 Browser ──HTTPS────► │  app (Next.js :3001) ──rewrites──► backend (Drogon :3000)     │
                      │                                        │                      │
 ESP32 ───HTTP:3000─► │  (porta publicada no host) ────────────┤                      │
                      │                                        ▼                      │
                      │                                  mysql (:3306, só interno)    │
                      └───────────────────────────────────────────────────────────────┘
```

Separar front e back é necessário porque:

1. **O ESP32 fala direto com o backend** (`POST /data`, `GET /config`) por HTTP
   cru — ele não passa pelo Next. O backend precisa de exposição própria.
2. O browser **nunca** chama o backend diretamente: o Next reescreve
   `/auth`, `/conections`, `/dispositivos`, `/hardware`, `/analytics` e `/health`
   para o backend pela **rede interna** (same-origin → sem CORS e o cookie
   HttpOnly de refresh funciona).
3. Builds e ciclos de vida diferentes (C++/CMake vs Node) — rebuild de um não
   derruba o outro.

## Arquivos criados para o deploy

| Arquivo | Papel |
|---|---|
| `backend/Dockerfile` | Compila Drogon + backend; imagem final enxuta (~240 MB). |
| `backend/docker/entrypoint.sh` | Gera `config/app.config.json` a partir de env vars, espera o MySQL e cria o schema na 1ª subida (`DB_AUTO_INIT`). |
| `app/Dockerfile` | Build standalone do Next.js 16. |
| `docker-compose.yml` | Teste local da stack inteira (e alternativa "Compose" no EasyPanel). |
| `.env.example` | Modelo de variáveis para o compose local. |

## Passo a passo no EasyPanel

Os exemplos assumem o projeto chamado **`sixseven`**. O DNS interno do
EasyPanel é `<projeto>_<serviço>` — se usar outro nome de projeto, ajuste os
hosts abaixo.

### 1. Projeto e banco

1. Crie o projeto `sixseven`.
2. **+ Service → MySQL**, nome **`mysql`** (imagem 8.x), defina a senha do
   root. Não exponha porta pública. Host interno: `sixseven_mysql`.

### 2. Backend

**+ Service → App**, nome **`backend`**:

- **Source**: GitHub → este repositório, branch `main`, **Build Path: `/backend`**.
- **Build**: Dockerfile (arquivo `Dockerfile`, relativo ao Build Path).
- **Environment**:

  ```env
  DB_HOST=sixseven_mysql
  DB_PORT=3306
  DB_NAME=SixSeven_Projects
  DB_USER=root
  DB_PASSWORD=<senha do mysql acima>
  JWT_SECRET=<gere com: openssl rand -hex 32>
  APP_ENV=production
  CORS_ALLOW_ORIGIN=*
  DB_AUTO_INIT=true
  DB_SEED=true
  ```

- **Ports (para o ESP32)**: em *Advanced/Ports*, publique **3000 → 3000 (TCP)**.
  O firmware usa HTTP cru em `IP:porta` — é este o caminho dele, não o domínio
  HTTPS (o redirect 80→443 quebraria o POST do ESP32).
- **Domains (opcional)**: `api.seudominio.com` → porta `3000`, se quiser a API
  com HTTPS para outros clientes.
- Deploy. Na primeira subida o entrypoint roda os SQLs de `backend/DB/` na
  ordem certa (config → tables → views → functions → CRUD → logic → triggers
  → data). `DB_SEED=false` pula o `data.sql` (dados de demonstração, inclui o
  usuário de teste `arthur@sixseven.dev` / `123456` — **não deixe em produção
  de verdade**).

### 3. Front

**+ Service → App**, nome **`app`**:

- **Source**: mesmo repositório, **Build Path: `/app`**.
- **Build**: Dockerfile.
- **Environment** (⚠️ defina **antes do primeiro build** — ver nota abaixo):

  ```env
  BACKEND_URL=http://sixseven_backend:3000
  ```

- **Domains**: seu domínio principal → porta **3001**.
- Deploy.

> **⚠️ `BACKEND_URL` é resolvido no BUILD, não no runtime.** Os `rewrites` do
> `next.config.ts` são serializados durante o `next build`. Se mudar a URL do
> backend, clique em **Rebuild** no serviço `app`. Se o seu EasyPanel não
> repassar variáveis de ambiente como build-arg, edite o default do `ARG
> BACKEND_URL` em `app/Dockerfile` (hoje: `http://sixseven_backend:3000`).

### 4. Verificação

```bash
curl https://api.seudominio.com/health        # ou http://IP-DA-VPS:3000/health
# {"ok":true,"data":{"status":"up",...}}
```

Abra o domínio do front, faça login com o usuário do seed (se `DB_SEED=true`)
e confira o dashboard.

### 5. Apontar o ESP32 para a VPS

Em `SixSeven_Projects/config.hpp` (firmware):

```cpp
#define SERVER_HOST "IP-DA-VPS"   // ou domínio, mas SEM https
#define SERVER_PORT 3000          // porta publicada no passo 2
```

Regrave o firmware. O `CONNECTION_ID` precisa existir na tabela `conections`
(o seed cria a conexão de id 1).

## Teste local (mesmas imagens do deploy)

```bash
cp .env.example .env    # ajuste MYSQL_ROOT_PASSWORD e JWT_SECRET
docker compose up --build
# front:   http://localhost:3001
# backend: http://localhost:3000/health
```

Alternativa no EasyPanel: em vez de 3 serviços, um único serviço **Compose**
apontando para o `docker-compose.yml` da raiz também funciona — mas com
serviços separados o painel dá logs, deploy e domínio individuais, o que é
mais prático no dia a dia.

## Notas de produção

- **Segredos**: nada vai para a imagem — `config/app.config.json` é gerado no
  boot pelo entrypoint e está no `.gitignore`/`.dockerignore`.
- **Cookie de refresh**: com o front atrás de HTTPS, vale habilitar a linha
  comentada `cookie.setSecure(true)` no `AuthController` do backend (exige
  rebuild).
- **Backup do MySQL**: use o backup nativo do serviço MySQL do EasyPanel — o
  dado persistente do sistema mora todo ali.
- **Logs do backend**: vão para stdout do Drogon e para `storage/logs` dentro
  do container (efêmero). Os logs do painel cobrem o dia a dia.
