# SixSeven_Projects — Backend

Backend em **C++** (framework **Drogon**) + **MySQL** para um sistema web que
**mostra e opera hardware** (medidores de energia baseados em ESP32). O
front-end é feito à parte em **React** e consome esta API.

O sistema é o intermediário entre o site e o hardware: o site pede dados e
manda configurações; o backend guarda tudo no banco; o hardware envia leituras
e busca a configuração. Este README explica **como rodar do zero**, como o
projeto está organizado, **por que** cada decisão foi tomada, e lista **todas
as rotas**.

---

## Sumário

1. [Visão geral e fluxo](#1-visão-geral-e-fluxo)
2. [Por que Drogon](#2-por-que-drogon)
3. [Pré-requisitos e instalação](#3-pré-requisitos-e-instalação)
4. [Banco de dados: ordem de execução](#4-banco-de-dados-ordem-de-execução)
5. [Configuração](#5-configuração)
6. [Compilar e rodar](#6-compilar-e-rodar)
7. [Arquitetura e padrões de projeto](#7-arquitetura-e-padrões-de-projeto)
8. [Estrutura de pastas](#8-estrutura-de-pastas)
9. [Autenticação e segurança](#9-autenticação-e-segurança)
10. [O contrato do hardware (não alterar)](#10-o-contrato-do-hardware-não-alterar)
11. [Rotas](#11-rotas)
12. [Teste rápido (curl)](#12-teste-rápido-curl)
13. [Notas e próximos passos](#13-notas-e-próximos-passos)

---

## 1. Visão geral e fluxo

Cada **conexão** (`conections`) representa **um hardware**. O hardware (ESP32)
é um **cliente HTTP** que, a cada intervalo (`timer`), faz **duas** chamadas:

```
[ESP32] --POST /data-->        [BACKEND]  grava em: info + info_historico
[ESP32] <--GET /config?id=--   [BACKEND]  lê de: info (E_b, timer)

[SITE]  --POST /hardware/{id}/config-->   grava E_b/timer (o ESP32 puxa depois)
[SITE]  --GET  /hardware/{id}/info----->  última leitura detalhada
[SITE]  --GET  /analytics/{id}/dashboard-> gráficos + resumo estatístico
```

Ponto importante de arquitetura: **o backend nunca abre conexão para o
hardware**. Quando o site "manipula o hardware", na verdade grava a nova
configuração (`E_b`, `timer`) no banco; o hardware busca isso sozinho na
próxima chamada `GET /config`. Isso bate exatamente com o firmware e cumpre o
fluxo pedido (site → backend → hardware, e hardware → backend), com o backend
sempre no meio.

**Significado dos campos** (vindos do firmware):

| Campo | Significado |
|-------|-------------|
| `E_b`   | Energia base (referência configurada pelo site). |
| `E_r`   | Energia recebida (leitura atual). |
| `timer` | Intervalo de envio, em milissegundos. |
| `M_er`  | Média da energia recebida (média móvel). |
| `V_mer` | Variação (distância entre `E_r` e a média). |
| `M_vmer`| Média das variações. |

Estabilidade: o firmware considera estável quando `V_mer <= 150`. O backend
replica esse limiar (constante `STABLE_THRESHOLD = 150`).

---

## 2. Por que Drogon

**Drogon** é um framework web C++ assíncrono (baseado em event loop, estilo
epoll/kqueue). Foi escolhido porque entrega, numa só peça, tudo que o projeto
precisa:

- **Servidor HTTP de alto desempenho** com modelo assíncrono — segura muitas
  conexões simultâneas (vários hardwares + o site) sem uma thread por conexão.
- **ORM/DbClient nativo para MySQL** com **prepared statements** (proteção
  contra SQL injection) e suporte a chamar **stored procedures**, que é como
  este projeto acessa o banco.
- **Controllers com roteamento por macros** (`METHOD_LIST`), que casa
  perfeitamente com o padrão **MVC** pedido.
- **Filtros (middleware)** de primeira classe — usados aqui para **CORS** e
  **autenticação JWT**.
- **JSON integrado** (JsonCpp) e utilidades de **cookie**, essenciais para a
  estratégia de sessão.

Em resumo: dá para construir um backend MVC completo, seguro e rápido sem
costurar cinco bibliotecas diferentes.

Como o Drogon funciona, em uma frase: você declara *controllers* cujas funções
são associadas a rotas; quando chega uma requisição, o event loop despacha para
a função certa (passando por eventuais *filtros*), e você responde de forma
assíncrona via um callback.

---

## 3. Pré-requisitos e instalação

Você precisa de: **CMake ≥ 3.16**, compilador **C++17**, **Drogon**,
**OpenSSL**, **cliente MySQL** e o **servidor MySQL/MariaDB**.

### Instalação automática (Ubuntu/Debian — recomendado)

Um script faz tudo de uma vez: instala dependências do sistema, compila e
instala o Drogon, baixa a libbcrypt e compila o backend:

```bash
chmod +x install.sh && ./install.sh
```

### Instalação manual (Ubuntu 25.04 / Debian)

**1) Dependências do sistema:**

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential cmake git \
    libjsoncpp-dev uuid-dev zlib1g-dev libssl-dev \
    libpq-dev libbrotli-dev libmysqlclient-dev
```

> Pacotes que já costumam vir instalados: `libjsoncpp-dev`, `uuid-dev`,
> `zlib1g-dev`, `libssl-dev`. Os que normalmente **faltam** e causam erro no
> cmake: `libpq-dev`, `libbrotli-dev`, `libmysqlclient-dev`.

**2) Drogon (compilar da fonte — única forma confiável no Ubuntu 25.04):**

```bash
git clone --depth=1 https://github.com/drogonframework/drogon
cd drogon && git submodule update --init --recursive
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_MYSQL=ON -DBUILD_POSTGRESQL=ON -DBUILD_SQLITE=OFF
make -j$(nproc)
sudo make install
sudo ldconfig
```

> Alternativa macOS: `brew install drogon openssl mysql-client`

**3) libbcrypt (vendorizada em `composers/`):**

```bash
cd composers
git clone --depth=1 https://github.com/rg3/libbcrypt.git
mkdir -p bcrypt && cp libbcrypt/bcrypt.h bcrypt/bcrypt.h
cd ..
```

Detalhes e alternativa com Argon2 em [`composers/README.md`](composers/README.md).

---

## 4. Banco de dados: ordem de execução

Os arquivos SQL ficam em `DB/` e **devem ser executados nesta ordem** (cada um
depende do anterior):

```
1) config.sql     -> cria o database e ajusta a sessão
2) tables.sql     -> tabelas (inclui info_historico e refresh_token)
3) views.sql      -> visões de leitura (index_*)
4) functions.sql  -> funções auxiliares (validação, estabilidade, %)
5) CRUD.sql       -> procedures de CRUD por tabela
6) logic.sql      -> camada de serviço do banco (dispatchers + info_Push)
7) triggers.sql   -> auditoria automática (tabela auditoria)
8) data.sql       -> dados de exemplo (usuário, conexões, histórico p/ gráficos)
```

Rodando tudo de uma vez (Linux/macOS):

```bash
cd DB
for f in config tables views functions CRUD logic triggers data; do
  echo ">> $f.sql"
  mysql -u root -p < "$f.sql"
done
```

Ou uma linha só:

```bash
cat DB/config.sql DB/tables.sql DB/views.sql DB/functions.sql \
    DB/CRUD.sql DB/logic.sql DB/triggers.sql DB/data.sql | mysql -u root -p
```

**Usuário de teste** criado pelo `data.sql`:
`arthur@sixseven.dev` / senha `123456`.

### O que foi corrigido no SQL (mantendo as tabelas)

O material original tinha erros que impediam o banco de subir. Foram
corrigidos, preservando colunas e dados:

- **`dispositivos`**: a FK apontava para uma coluna inexistente
  (`id_conection`); corrigida para `id_usuario → usuario(id)`.
- **`views`**: vírgulas sobrando antes de `FROM`, nome `concetions` (typo) e
  colunas inexistentes (`marca`, `modelo`, `can_Active` em `dispositivos`);
  as visões foram reescritas conforme as tabelas reais. A visão de usuário
  **não expõe a senha**.
- **`usuario.senha`** ampliada de `VARCHAR(32)` para `VARCHAR(255)` para caber
  o hash **bcrypt** (a de 32 sugeria MD5, inseguro).

### O que foi acrescentado

- **`info_historico`**: série temporal (com `criado_em`) alimentada a cada
  `POST /data`. É a base dos **gráficos**. A tabela `info` continua guardando
  apenas a **última** leitura por hardware (1 linha por conexão), como antes.
- **`refresh_token`**: sessão persistente ("manter logado"). Guarda apenas o
  **hash** do refresh token, nunca o valor cru.

---

## 5. Configuração

Toda a infraestrutura fica em [`config/app.config.json`](config/app.config.json):
porta, threads, log, **conexão do banco** (`db_clients`) e o bloco
**`security`** (segredo do JWT, TTLs, custo do bcrypt, origem de CORS).

- O código de negócio lê o bloco `security` via `app/Config/AppConfig.hpp`.
- O resto (listeners, banco, log) é lido pelo próprio Drogon.

**Antes de rodar**, ajuste:

```jsonc
"db_clients": [{ ..., "user": "root", "passwd": "SUA_SENHA_DO_MYSQL" }],
"security": {
  "jwt_secret": "GERE_UMA_CHAVE_LONGA",   // ex.: openssl rand -hex 32
  "cors_allow_origin": "http://localhost:5173"  // origem do seu React
}
```

Há um `config/app.config.example.json` versionável (sem segredos). **Não
commite segredos reais** — o `.gitignore` já cuida de arquivos locais.

---

## 6. Compilar e rodar

```bash
# na raiz do backend
mkdir build && cd build
cmake ..
make -j$(nproc)

# o CMake copia config/ e public/ e cria storage/logs ao lado do binário
./sixseven_backend
```

O servidor sobe em `http://0.0.0.0:3000` (ajustável no JSON). Teste:

```bash
curl http://localhost:3000/health
# {"ok":true,"data":{"status":"up","app":"SixSeven_Projects","env":"development"}}
```

> Para instalar todas as dependências e compilar de uma vez, use o script
> `install.sh` na raiz do backend (ver seção 3).

---

## 7. Arquitetura e padrões de projeto

O backend segue **MVC** com uma separação em camadas clara, do HTTP até o banco:

```
HTTP  ─▶ Controller ─▶ Service ─▶ Repository ─▶ (Stored Procedures) ─▶ MySQL
                 ▲          ▲            ▲
              Helpers    regras de     acesso a
              (JSON,     negócio       dados
              parsing)   + posse
```

- **Controllers** (`app/Controllers`): só orquestram entrada/saída HTTP.
  Escrevem apenas o "caminho feliz"; erros viram JSON no `BaseController`.
- **Services** (`app/Service`): regras de negócio (posse de conexão,
  autenticação, analytics). Nada de SQL aqui.
- **Repositories** (`app/Repository`): acesso a dados, **sempre** via
  **stored procedures** com prepared statements. O banco é a fonte de verdade.
- **Models** (`app/Models`): entidades de domínio + serialização JSON.

### Padrões aplicados (e onde)

- **MVC** — organização geral (Controllers/Models + Services).
- **Repository** — `app/Repository/*` isola a persistência do resto.
- **Service Layer** — `app/Service/*` concentra regras de negócio.
- **Singleton** — `AppConfig` (uma fonte única de configuração em runtime).
- **Facade** — `Config/Database.hpp` esconde o `DbClient` do Drogon atrás de
  um ponto único.
- **Strategy** — `AnalyticsService`: cada **gráfico** é uma estratégia
  (`EnergiaVsMediaStrategy`, `VariacaoStrategy`, `PercentualStrategy`).
  **Adicionar um gráfico novo = adicionar uma classe**, sem tocar no resto.
  Foi assim que implementei "várias funções para mostrar os dados de forma
  interessante" de um jeito extensível.
- **Template Method** — `BaseController::handle()` define o esqueleto
  (executar → capturar exceção → responder JSON) reutilizado por todos.
- **Chain of Responsibility** — os **filtros** do Drogon (CORS → Auth) formam
  a cadeia que a requisição percorre antes do controller.
- **DTO / envelope** — toda resposta do site usa `{ok, data|error}`.

### Como as camadas de segurança se encaixam

A pasta `app/Security` tem métodos **independentes** (cada um faz uma coisa) e
o **tratamento de rota** fica nos **middlewares**:

- `PasswordHasher` (bcrypt), `Hash` (SHA-256/aleatório), `Jwt` (HS256),
  `Sanitizer` (validação de entrada). Nenhum depende de HTTP.
- `AuthMiddleware`/`CorsMiddleware` aplicam essas peças no fluxo das rotas.

---

## 8. Estrutura de pastas

```
backend/
├── CMakeLists.txt            # build (Drogon + libbcrypt)
├── main.cpp                  # ponto de entrada (carrega config, sobe servidor)
├── DB/                       # SQL em camadas (ver seção 4)
│   ├── config.sql  tables.sql  views.sql  functions.sql
│   └── CRUD.sql    logic.sql   triggers.sql  data.sql
├── config/                   # config FORA da app (infra + segredos)
│   ├── app.config.json
│   └── app.config.example.json
├── bootstrap/
│   └── Bootstrap.hpp         # registra filtros/controllers e 404 padrão
├── composers/                # dependências vendorizadas (libbcrypt)
│   └── README.md
├── public/                   # arquivos estáticos (landing simples da API)
├── storage/                  # logs e uploads em runtime
│   ├── logs/  uploads/
└── app/                      # o coração (MVC)
    ├── Config/               # AppConfig (Singleton) + Database (Facade)
    ├── Controllers/          # Health, Auth, Conection, Dispositivo,
    │                         # Hardware, Analytics + BaseController
    ├── Service/              # Auth, Conection, Dispositivo, Hardware, Analytics
    ├── Repository/           # 1 repo por tabela (procedures)
    ├── Models/               # entidades + toJson/fromRow
    ├── Middleware/           # CorsMiddleware, AuthMiddleware (filtros Drogon)
    ├── Helpers/              # Response (envelope JSON), RequestParser
    ├── Exceptions/           # ApiException (mapeia p/ status HTTP)
    ├── Security/             # PasswordHasher, Hash, Jwt, Sanitizer
    └── Routes/
        └── routes.md         # mapa único de todas as rotas
```

---

## 9. Autenticação e segurança

Estratégia de sessão pensada para **manter o usuário logado** com segurança:

- **Access token = JWT (HS256)**, curto (padrão **15 min**). Vai no corpo da
  resposta; o front envia em `Authorization: Bearer <token>`.
- **Refresh token = string opaca** aleatória, longa (padrão **30 dias**).
  Guardamos apenas o **SHA-256** dele no banco (`refresh_token`), nunca o valor
  cru. Vai num **cookie HttpOnly** (mais seguro que `localStorage`, imune a
  XSS de leitura) e também no corpo, para clientes não-browser.
- **Rotação**: a cada `POST /auth/refresh`, o token antigo é **revogado** e um
  novo par é emitido. Roubo de um refresh usado fica evidente.
- **Senhas** com **bcrypt** (salt por senha, custo configurável). O
  `PasswordHasher` é um ponto de troca (dá para migrar a Argon2 sem mexer no
  resto — Strategy).
- **SQL injection**: todo acesso usa **prepared statements** + **stored
  procedures**. O `Sanitizer` adiciona uma camada extra (defense-in-depth),
  rejeitando entradas obviamente inválidas cedo.
- **CORS** restrito à origem do front (configurável).
- **Mensagens genéricas** no login (não revela se o e-mail existe).

---

## 10. O contrato do hardware (não alterar)

As rotas que o **firmware** usa têm contrato fixo e **não devem ser
modificadas** (o ESP32 já está programado para elas). Elas **não usam
autenticação** e **não usam o envelope** `{ok,data}` — respondem o JSON cru:

- **`POST /data`** — corpo enviado pelo hardware:
  ```json
  { "id":1, "E_b":3000, "E_r":2850, "timer":10000,
    "M_er":2820, "V_mer":90, "M_vmer":75 }
  ```
  O backend grava a leitura (`info`) e um ponto no histórico
  (`info_historico`), numa transação. Responde `{ "ok": true }`. Preserva o
  `E_b`/`timer` configurados pelo site (o hardware manda leitura, o site manda
  configuração).

- **`GET /config?id=N`** — o backend responde exatamente:
  ```json
  { "id":1, "E_b":3000, "timer":10000 }
  ```
  O firmware lê de volta `id`, `E_b` e `timer` (e nada mais). Se ainda não há
  configuração, retornam defaults seguros (`E_b:0`, `timer:10000`).

> Se um dia o hardware precisar de autenticação (ex.: token por dispositivo),
> isso será um acréscimo **compatível**, sem quebrar o shape atual.

---

## 11. Rotas

Resumo abaixo; a **fonte única** (sempre atualizada com os controllers) é
[`app/Routes/routes.md`](app/Routes/routes.md).

### Público
| Método | Rota | Descrição |
|---|---|---|
| GET | `/health` | Status do serviço. |

### Autenticação
| Método | Rota | Auth | Corpo |
|---|---|---|---|
| POST | `/auth/register` | não | `{nome,email,senha}` |
| POST | `/auth/login` | não | `{email,senha}` |
| POST | `/auth/refresh` | cookie/refresh | — (usa cookie `refresh_token`) |
| POST | `/auth/logout` | cookie/refresh | — |
| GET | `/auth/me` | sim | — |

### Conexões (hardwares) — Auth
| Método | Rota | Descrição |
|---|---|---|
| GET | `/conections` | Lista as conexões do usuário. |
| POST | `/conections` | `{nome, can_active?}` cria. |
| GET | `/conections/{id}` | Detalhe (valida posse). |
| PUT | `/conections/{id}` | `{nome, can_active?, is_active?}` atualiza. |
| DELETE | `/conections/{id}` | Remove. |

### Dispositivos — Auth
| Método | Rota | Descrição |
|---|---|---|
| GET | `/dispositivos` | Lista. |
| POST | `/dispositivos` | `{nome, tipo}` cria. |
| DELETE | `/dispositivos/{id}` | Remove. |
| GET | `/dispositivos/links` | Lista relações N:N. |
| POST | `/dispositivos/link` | `{id_conection, id_dispositivo}` associa. |
| POST | `/dispositivos/unlink` | `{id_conection, id_dispositivo}` desassocia. |

### Hardware
| Método | Rota | Auth | Descrição |
|---|---|---|---|
| POST | `/data` | não | **Firmware** envia leitura (shape cru). |
| GET | `/config?id=N` | não | **Firmware** puxa `{id,E_b,timer}`. |
| GET | `/hardware/{id}/info` | sim | **Site**: última leitura + derivadas. |
| POST | `/hardware/{id}/config` | sim | **Site**: grava `{E_b,timer}` (manipular). |

### Gráficos / Métricas — Auth
| Método | Rota | Descrição |
|---|---|---|
| GET | `/analytics/{id}/dashboard?limite=N` | Todos os gráficos + resumo. |
| GET | `/analytics/charts` | Lista de gráficos disponíveis. |

O `dashboard` devolve, para cada gráfico, um `dataset` no formato
`{ labels:[...], series:[{name,data:[...]}] }` — pronto para bibliotecas como
Recharts/Chart.js — mais um `resumo` com min/máx/média de `E_r`, `%` de tempo
estável e o estado da última leitura.

---

## 12. Teste rápido (curl)

```bash
# 1) login (usuário do seed)
curl -s -c cookies.txt -X POST http://localhost:3000/auth/login \
  -H "Content-Type: application/json" \
  -d '{"email":"arthur@sixseven.dev","senha":"123456"}'

# copie o access_token da resposta:
TOKEN="cole_o_access_token_aqui"

# 2) minhas conexões
curl -s http://localhost:3000/conections -H "Authorization: Bearer $TOKEN"

# 3) dashboard de gráficos da conexão 1
curl -s "http://localhost:3000/analytics/1/dashboard?limite=50" \
  -H "Authorization: Bearer $TOKEN"

# 4) simular o hardware enviando uma leitura
curl -s -X POST http://localhost:3000/data \
  -H "Content-Type: application/json" \
  -d '{"id":1,"E_b":3000,"E_r":2900,"timer":10000,"M_er":2830,"V_mer":70,"M_vmer":72}'

# 5) simular o hardware puxando a config
curl -s "http://localhost:3000/config?id=1"

# 6) site manipulando o hardware (nova base/intervalo)
curl -s -X POST http://localhost:3000/hardware/1/config \
  -H "Authorization: Bearer $TOKEN" -H "Content-Type: application/json" \
  -d '{"E_b":3200,"timer":8000}'
```

---

## 13. Notas e próximos passos

- **Compilar antes de usar em produção.** Ver a nota de transparência na seção 6.
- **HTTPS**: em produção, sirva atrás de TLS e habilite `cookie.setSecure(true)`
  no `AuthController` (linha comentada).
- **`AuthService::refresh`** hoje resolve o dono do token varrendo o índice de
  usuários. Para muitos usuários, vale adicionar uma procedure
  `usuario_FindById` e trocar a busca (é uma otimização localizada).
- **Limpeza de tokens**: um job periódico para apagar `refresh_token`
  expirados/revogados mantém a tabela enxuta.
- **Front (React)**: guarde o `access_token` em memória e deixe o refresh no
  cookie HttpOnly; chame `/auth/refresh` quando o access expirar (401).
- **Novos gráficos**: crie uma nova `ChartStrategy` em `AnalyticsService.hpp` e
  registre no construtor — aparece automaticamente no dashboard e em
  `/analytics/charts`.
```
