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

O DNS interno do EasyPanel é **`<projeto>_<serviço>`**. Os exemplos abaixo
usam o projeto real **`puc`** com os serviços `mysql`, `sixseven_backend` e
`sixseven_app` — se os nomes forem outros, ajuste os hosts na mesma regra.

### 1. Projeto e banco

1. Crie o projeto `puc` (ou use o existente).
2. **+ Service → MySQL** (imagem 8.x). Não exponha porta pública. O template
   cria um database e um usuário de aplicação (ex.: database
   `sixseven_mysql`, usuário `mysql`) — dá para usá-los direto no backend,
   **desde que** o binlog seja desligado: no serviço MySQL, defina o
   **Command** (Advanced) como `mysqld --skip-log-bin` e reinicie. Sem isso,
   o MySQL 8 exige SUPER para criar as functions/triggers do schema e o init
   falha com `ERROR 1419` (só o root passaria). Binlog não é usado neste
   projeto e ainda economiza disco.

### 2. Backend

**+ Service → App**, nome **`sixseven_backend`**:

- **Source**: GitHub → este repositório, branch `main`, **Build Path: `/backend`**.
- **Build**: Dockerfile (arquivo `Dockerfile`, relativo ao Build Path).
- **Environment**:

  ```env
  DB_HOST=<host interno do serviço MySQL, ex.: sixseven_mysql>
  DB_PORT=3306
  DB_NAME=sixseven_mysql
  DB_USER=mysql
  DB_PASSWORD=<senha do usuário mostrada no serviço MySQL>
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

**+ Service → App**, nome **`sixseven_app`**:

- **Source**: mesmo repositório, **Build Path: `/app`**.
- **Build**: Dockerfile.
- **Environment** (⚠️ defina **antes do primeiro build** — ver nota abaixo):

  ```env
  BACKEND_URL=http://puc_sixseven_backend:3000
  ```

- **Domains**: seu domínio principal → porta **3001**.
- Deploy.

> **⚠️ `BACKEND_URL` é resolvido no BUILD, não no runtime.** Os `rewrites` do
> `next.config.ts` são serializados durante o `next build`. Se mudar a URL do
> backend, clique em **Rebuild** no serviço do front. O EasyPanel repassa as
> variáveis de ambiente do serviço como `--build-arg` (confirmado nos logs de
> build), então basta manter a env correta — o valor tem que ser
> `http://<projeto>_<serviço-backend>:3000`.

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

## Troubleshooting

### Build do front morre em `npm ci` com `exit code: 152` (ou 137)

Exit code acima de 128 significa **processo morto por sinal** (código − 128):
**152 = SIGXCPU** (limite de tempo de CPU excedido) e **137 = SIGKILL**
(normalmente o OOM killer, falta de RAM). O build roda **na própria VPS** —
em máquinas pequenas (1 vCPU / 1–2 GB) o `npm ci`/`next build` estoura o
limite. Na ordem:

1. **Tente de novo** (Force Rebuild). Se a VPS estava momentaneamente
   sobrecarregada, passa na segunda.
2. **Confira RAM e swap** na VPS: `free -h`. Sem swap, crie 2 GB:

   ```bash
   fallocate -l 2G /swapfile && chmod 600 /swapfile
   mkswap /swapfile && swapon /swapfile
   echo '/swapfile none swap sw 0 0' >> /etc/fstab
   ```

3. **Confira se foi OOM ou limite do Docker**: `dmesg | grep -iE "oom|killed"`
   mostra mortes por memória; `cat /etc/docker/daemon.json` — se houver
   `default-ulimits` com `cpu`, é ele que envia o SIGXCPU (remova e
   `systemctl restart docker`).
4. **Plano B (sempre funciona em VPS fraca)**: buildar a imagem fora da VPS —
   GitHub Actions publicando em `ghcr.io` — e trocar o Source do serviço no
   EasyPanel de "GitHub" para "Docker Image" apontando para a imagem pronta.
   A VPS passa a só fazer `pull`.

### Backend em loop de restart com `ERROR 1044` ou `ERROR 1419` no init

Dois erros da mesma família — o usuário de `DB_USER` sem privilégio
suficiente. Nenhum precisa de console no backend (ele se recupera sozinho no
próximo restart, desfazendo qualquer schema parcial):

- **`ERROR 1044 Access denied ... to database 'SixSeven_Projects'`**: o
  usuário do template só enxerga o database do próprio template. Aponte
  `DB_NAME` para **esse** database (ex.: `sixseven_mysql`) — o entrypoint
  reescreve o nome nos SQLs automaticamente. (Ou use `DB_USER=root`.)
- **`ERROR 1419 You do not have the SUPER privilege and binary logging is
  enabled`**: o binlog do MySQL 8 exige SUPER para criar functions/triggers.
  Correção: no **serviço MySQL**, Command = `mysqld --skip-log-bin` e
  restart (testado — o init completo passa com o usuário comum). Alternativa
  sem mexer no command: no console do **serviço MySQL** (esse não está em
  loop), rode como root `SET GLOBAL log_bin_trust_function_creators = 1;` e
  reinicie o backend — vale até o próximo restart do MySQL.

### Como dar seed no banco na VPS

Na **primeira subida** com `DB_AUTO_INIT=true` e `DB_SEED=true` o seed é
automático (o entrypoint roda `DB/data.sql` junto com o schema). Se o banco
**já existe** (o entrypoint loga "Schema já existe — pulando init"), há dois
caminhos, ambos pelo **Console** do serviço `sixseven_backend` no EasyPanel —
a imagem já traz o cliente `mysql`, os SQLs em `DB/` e as credenciais nas
envs:

**Só o seed** (banco criado sem dados — rode UMA vez, o `data.sql` é
`INSERT` puro e duplica se repetido; o `sed` ajusta o nome do database,
igual o entrypoint faz):

```bash
sed "s/SixSeven_Projects/$DB_NAME/g" DB/data.sql | \
  mysql -h"$DB_HOST" -P"$DB_PORT" -u"$DB_USER" -p"$DB_PASSWORD"
```

**Reset completo** (⚠️ apaga TUDO, inclusive leituras já enviadas pelo ESP32):

```bash
mysql -h"$DB_HOST" -P"$DB_PORT" -u"$DB_USER" -p"$DB_PASSWORD" \
  -e "DROP DATABASE \`$DB_NAME\`"
```

Depois clique em **Restart** no serviço — o entrypoint recria schema + seed.

Para conferir se o seed já está lá:

```bash
mysql -h"$DB_HOST" -P"$DB_PORT" -u"$DB_USER" -p"$DB_PASSWORD" \
  -e "SELECT id, email FROM \`$DB_NAME\`.usuario"
```

### Front no ar mas login/API falham (502/ECONNREFUSED nos rewrites)

Quase sempre é `BACKEND_URL` errado no build. O host interno é
`<projeto>_<serviço>` — confira o nome exato do serviço do backend no painel
e **Rebuild** no front após corrigir a env.

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
