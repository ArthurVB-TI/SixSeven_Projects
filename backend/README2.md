# SixSeven_Projects — Backend (versão simples: httplib + MySQL)

Backend em **C++17** que **mostra e opera hardware** (medidores de energia
ESP32). Esta versão foi feita para ser **fácil de compilar**: em vez do Drogon
(que dá muito trabalho de instalar), usa a biblioteca **cpp-httplib**, que é
**um único arquivo** — sem instalar framework, sem `make install`, sem
`ldconfig`.

O front-end (React) é feito à parte e consome esta API.

> **Por que trocamos o Drogon?** C++ não tem servidor HTTP na biblioteca
> padrão — sempre é preciso uma lib externa. O Drogon é ótimo, mas exige
> compilar o framework e várias dependências, o que estava dando erro. O
> **httplib** entrega o mesmo (servidor HTTP) num header só. Menos poder bruto,
> muito menos chance de erro.

---

## O que você precisa instalar

Bem menos que antes. No **Ubuntu/Debian**:

```bash
sudo apt update
sudo apt install -y build-essential cmake git \
    libssl-dev libmysqlcppconn-dev mysql-server
```

- `build-essential` + `cmake` → compilador e build.
- `libssl-dev` → OpenSSL (usado no JWT e nos hashes).
- `libmysqlcppconn-dev` → **MySQL Connector/C++** (o backend fala com o MySQL).
- `mysql-server` → o banco (se ainda não tiver; você mencionou usar o Workbench,
  então provavelmente o servidor já existe).

E **dois arquivos header** que você baixa uma vez (são de um arquivo só):

### vendor/ (baixe 2 arquivos + a libbcrypt)

1. **httplib.h** — https://github.com/yhirose/cpp-httplib → salve em `vendor/httplib.h`
2. **json.hpp** — https://github.com/nlohmann/json/releases → salve em `vendor/json.hpp`
3. **libbcrypt** (hash de senha):
   ```bash
   cd vendor
   git clone https://github.com/rg3/libbcrypt.git
   mkdir -p bcrypt && cp libbcrypt/bcrypt.h bcrypt/bcrypt.h
   cd ..
   ```

Detalhes em [`vendor/README.md`](vendor/README.md).

---

## O banco de dados

Você tem duas formas de criar o banco. **Não precisa criar à mão** — o sistema
faz isso.

### Opção A — o próprio servidor cria (automático)

Quando você roda o backend, ele **verifica se o banco existe** e, se não
existir, **cria tudo sozinho** (roda os 8 arquivos SQL na ordem certa). É a
classe `app/Db/DbBootstrap.hpp`, chamada no `main.cpp`. Você só precisa ter o
MySQL ligado e as credenciais certas no `config/app.config.json`.

### Opção B — o script (antes de compilar, se quiser)

Se preferir preparar o banco por fora (ex.: testar antes de compilar o C++),
use o script:

```bash
# usa root sem senha por padrão; ajuste conforme seu MySQL
DB_USER=root DB_PASS=suasenha bash scripts/db_setup.sh
```

Ele verifica se o banco existe e, se não, cria e popula. Também imprime o
usuário de teste.

**Usuário de teste** (criado pelo seed): `arthur@sixseven.dev` / senha `123456`.

### Ordem dos arquivos SQL (para referência)

```
config → tables → views → functions → CRUD → logic → triggers → data
```

---

## Configuração

Tudo em [`config/app.config.json`](config/app.config.json): porta do servidor,
dados do **banco** (`db_clients`) e **segurança** (JWT, bcrypt, CORS).

Ajuste antes de rodar:

```jsonc
"db_clients": [{
  "host": "127.0.0.1", "port": 3306,
  "dbname": "SixSeven_Projects",
  "user": "root", "passwd": "SUA_SENHA_DO_MYSQL"
}],
"security": {
  "jwt_secret": "GERE_UMA_CHAVE_LONGA",       // ex.: openssl rand -hex 32
  "cors_allow_origin": "http://localhost:5173" // origem do seu React
}
```

---

## Compilar e rodar

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)

# o build copia config/, public/ e DB/ para junto do binário
./sixseven_backend
```

Ao subir, você verá algo como:

```
==============================================
 SixSeven_Projects backend
 Ambiente: development
==============================================
[DB] Banco 'SixSeven_Projects' nao encontrado. Criando do zero...
[DB]   -> executando config.sql
...
[DB] Banco criado e populado com sucesso.
[HTTP] Ouvindo em http://0.0.0.0:3000
```

---

## Verificar se o back está funcionando

**1) O servidor está de pé?**

```bash
curl http://localhost:3000/health
# {"ok":true,"data":{"status":"up","app":"SixSeven_Projects","env":"development"}}
```

Se responder isso, o servidor está no ar.

**2) O banco está conectado e as leituras funcionam?**

```bash
# login (usuário do seed)
curl -s -X POST http://localhost:3000/auth/login \
  -H "Content-Type: application/json" \
  -d '{"email":"arthur@sixseven.dev","senha":"123456"}'
```

Copie o `access_token` da resposta e:

```bash
TOKEN="cole_aqui"

# minhas conexões (hardwares)
curl -s http://localhost:3000/conections -H "Authorization: Bearer $TOKEN"

# gráficos da conexão 1
curl -s "http://localhost:3000/analytics/1/dashboard?limite=50" \
  -H "Authorization: Bearer $TOKEN"
```

**3) Simular o hardware (sem precisar do ESP32):**

```bash
# hardware enviando leitura
curl -s -X POST http://localhost:3000/data \
  -H "Content-Type: application/json" \
  -d '{"id":1,"E_b":3000,"E_r":2900,"timer":10000,"M_er":2830,"V_mer":70,"M_vmer":72}'
# -> {"ok":true}

# hardware puxando a config
curl -s "http://localhost:3000/config?id=1"
# -> {"id":1,"E_b":3000,"timer":10000}
```

Se o `/health` funciona mas o login/leituras falham, o problema é o **banco**
(credenciais no config, ou o MySQL não está ligado). A mensagem de erro do
servidor no terminal diz qual é o caso.

---

## Como está organizado (arquitetura)

Mesma arquitetura **MVC em camadas** da versão anterior — só a camada HTTP e a
de banco mudaram:

```
HTTP (httplib) ─▶ Router ─▶ Service ─▶ Repository ─▶ (Stored Procedures) ─▶ MySQL
                     │          │            │
                  HttpUtil   regras de     Db (Connector/C++,
                  (JSON,     negócio       prepared statements)
                  auth,      + posse
                  CORS)
```

- **`app/Http/Router.hpp`** — registra todas as rotas e chama os Services.
  Um wrapper central (`guard`) transforma erros em JSON `{ok:false,error}`.
- **`app/Http/HttpUtil.hpp`** — envelope de resposta, CORS, autenticação JWT,
  leitura de campos do corpo.
- **`app/Service/*`** — regras de negócio (autenticação, posse de conexão,
  analytics). **Não mudaram** em relação à versão anterior (só o JSON).
- **`app/Repository/*`** — acesso a dados via **stored procedures** com
  **prepared statements**.
- **`app/Db/Db.hpp`** — conexão MySQL (Connector/C++), uma por thread.
- **`app/Db/DbBootstrap.hpp`** — cria o banco se não existir.
- **`app/Models/*`, `app/Security/*`, `app/Exceptions/*`** — entidades, JWT/
  bcrypt/hash, e erros. Praticamente iguais à versão anterior.

### Padrões de projeto (mantidos)

**MVC**, **Repository**, **Service Layer**, **Singleton** (`AppConfig`),
**Strategy** (os gráficos em `AnalyticsService` — cada gráfico é uma classe;
adicionar um novo não mexe no resto), e um **guard** central para erros
(Template Method).

---

## Rotas

Resumo abaixo; detalhes e formatos em [`app/Routes/routes.md`](app/Routes/routes.md).

| Método | Rota | Auth | Para quê |
|---|---|---|---|
| GET | `/health` | não | Status do serviço. |
| POST | `/auth/register` | não | Cria usuário. |
| POST | `/auth/login` | não | Login (access + refresh). |
| POST | `/auth/refresh` | cookie | Renova o access token. |
| POST | `/auth/logout` | cookie | Encerra a sessão. |
| GET | `/auth/me` | sim | Dados do usuário logado. |
| GET | `/conections` | sim | Lista conexões (hardwares) do usuário. |
| POST | `/conections` | sim | Cria conexão. |
| GET | `/conections/{id}` | sim | Detalhe (valida posse). |
| PUT | `/conections/{id}` | sim | Atualiza. |
| DELETE | `/conections/{id}` | sim | Remove. |
| GET | `/dispositivos` | sim | Lista dispositivos. |
| POST | `/dispositivos` | sim | Cria dispositivo. |
| DELETE | `/dispositivos/{id}` | sim | Remove. |
| GET | `/dispositivos/links` | sim | Relações N:N. |
| POST | `/dispositivos/link` | sim | Associa dispositivo↔conexão. |
| POST | `/dispositivos/unlink` | sim | Desassocia. |
| POST | `/data` | **não** | **Firmware** envia leitura (shape cru). |
| GET | `/config?id=N` | **não** | **Firmware** puxa `{id,E_b,timer}`. |
| GET | `/hardware/{id}/info` | sim | Site: última leitura detalhada. |
| POST | `/hardware/{id}/config` | sim | Site: grava `{E_b,timer}` (manipular). |
| GET | `/analytics/{id}/dashboard?limite=N` | sim | Gráficos + resumo. |
| GET | `/analytics/charts` | sim | Lista de gráficos disponíveis. |

O contrato do **firmware** (`/data` e `/config`) é **cru e sem autenticação**,
idêntico ao que o ESP32 espera — não foi alterado.

---

## Solução de problemas (o que provavelmente vai dar)

**`cmake` diz que não achou o MySQL Connector/C++**
→ Instale `libmysqlcppconn-dev`. Se ainda assim não achar, veja em qual pasta
ficou o header (`mysql_connection.h` / `jdbc.h`) e ajuste os caminhos no
`CMakeLists.txt` (variável de include e `find_library`).

**Erro `fatal error: httplib.h: No such file` ou `json.hpp`**
→ Você não baixou os dois arquivos em `vendor/`. Veja a seção "vendor/".

**Erro `bcrypt/bcrypt.h: No such file`**
→ Falta clonar a libbcrypt em `vendor/`. Veja "vendor/".

**Compila, mas ao rodar diz que não conectou no banco**
→ MySQL desligado ou senha errada em `config/app.config.json`. Rode
`bash scripts/db_setup.sh` para testar a conexão isoladamente.

**`/health` funciona, mas `/auth/login` dá erro 500**
→ O banco provavelmente não foi criado. Veja o log do terminal; rode o
`scripts/db_setup.sh` ou deixe o servidor criar (Opção A).

---

## Nota de transparência

Este código foi **validado por verificação de sintaxe** (compilação
`-fsyntax-only` de todo o grafo de arquivos, sem erros), mas **não foi
executado de ponta a ponta** no ambiente de geração, que não tinha MySQL nem
as bibliotecas de sistema instaladas. Ao compilar na sua máquina com as libs
acima, deve funcionar; se algo divergir por versão do Connector/C++, o ajuste
é nos caminhos do `CMakeLists.txt`.