# Mapa de Rotas — SixSeven_Projects

> No Drogon, cada rota é registrada **dentro do controller** pelo bloco
> `METHOD_LIST_BEGIN ... METHOD_LIST_END`. Este arquivo é a **fonte única
> de consulta** de todas as rotas — mantém o pedido de ter a estrutura de
> rotas visível em um só lugar. Ao adicionar uma rota no controller,
> atualize aqui também.

Todas as respostas do **site** usam o envelope padrão:

```json
{ "ok": true,  "data": { ... } }
{ "ok": false, "error": "mensagem" }
```

As rotas do **firmware** (`/data`, `/config`) são exceção: retornam o JSON
**cru** que o ESP32 espera, sem envelope e sem autenticação (contrato do
hardware, não alterar).

---

## Saúde (público)

| Método | Rota      | Auth | Descrição                        |
|--------|-----------|------|----------------------------------|
| GET    | `/health` | não  | Status do serviço (up/env/app).  |

## Autenticação — `AuthController`

| Método | Rota             | Auth | Corpo / Notas |
|--------|------------------|------|----------------|
| POST   | `/auth/register` | não  | `{nome,email,senha}` → cria usuário (201). |
| POST   | `/auth/login`    | não  | `{email,senha}` → `access_token` + refresh (cookie HttpOnly). |
| POST   | `/auth/refresh`  | não* | Usa cookie `refresh_token` (ou `{refresh_token}` no corpo). Rotaciona. |
| POST   | `/auth/logout`   | não* | Revoga o refresh atual e limpa o cookie. |
| GET    | `/auth/me`       | sim  | Dados do token (id/email). |

\* Não exige access token, mas depende do refresh token (cookie/corpo).

**Resposta de login/refresh (`data`):**
```json
{
  "access_token": "jwt...",
  "token_type": "Bearer",
  "expires_in": 900,
  "refresh_token": "opaco...",
  "usuario": { "id": 1, "nome": "...", "email": "..." }
}
```

## Conexões (hardwares) — `ConectionController` — todas com Auth

| Método | Rota                | Descrição |
|--------|---------------------|-----------|
| GET    | `/conections`       | Lista as conexões do usuário logado. |
| POST   | `/conections`       | `{nome, can_active?}` → cria (201, retorna `{id}`). |
| GET    | `/conections/{id}`  | Detalhe de uma conexão (valida posse). |
| PUT    | `/conections/{id}`  | `{nome, can_active?, is_active?}` → atualiza. |
| DELETE | `/conections/{id}`  | Remove a conexão. |

## Dispositivos — `DispositivoController` — todas com Auth

| Método | Rota                     | Descrição |
|--------|--------------------------|-----------|
| GET    | `/dispositivos`          | Lista dispositivos. |
| POST   | `/dispositivos`          | `{nome, tipo}` → cria (201, `{id}`). |
| DELETE | `/dispositivos/{id}`     | Remove dispositivo. |
| GET    | `/dispositivos/links`    | Lista relações N:N (conexão↔dispositivo). |
| POST   | `/dispositivos/link`     | `{id_conection, id_dispositivo}` → associa. |
| POST   | `/dispositivos/unlink`   | `{id_conection, id_dispositivo}` → desassocia. |

## Hardware — `HardwareController`

**(A) Firmware — SEM auth, shape cru (NÃO alterar):**

| Método | Rota               | Corpo / Query | Resposta |
|--------|--------------------|----------------|----------|
| POST   | `/data`            | `{id,E_b,E_r,timer,M_er,V_mer,M_vmer}` | `{ "ok": true }` |
| GET    | `/config?id=N`     | query `id`     | `{ "id":N, "E_b":.., "timer":.. }` |

**(B) Site — com Auth, com envelope, valida posse:**

| Método | Rota                       | Corpo | Descrição |
|--------|----------------------------|-------|-----------|
| GET    | `/hardware/{id}/info`      | —     | Última leitura + métricas derivadas (estável, %). |
| POST   | `/hardware/{id}/config`    | `{E_b, timer}` | Grava a config que o hardware puxa. |

## Gráficos / Métricas — `AnalyticsController` — todas com Auth

| Método | Rota                                    | Descrição |
|--------|-----------------------------------------|-----------|
| GET    | `/analytics/{id}/dashboard?limite=N`    | Todos os gráficos (Strategy) + resumo estatístico. |
| GET    | `/analytics/charts`                     | Lista de gráficos disponíveis (para montar abas). |

**Resposta do dashboard (`data`):**
```json
{
  "id_conection": 1,
  "pontos": 12,
  "charts": [
    { "key": "energia_vs_media", "label": "...",
      "dataset": { "labels": ["..."], "series": [ {"name":"E_r (recebida)","data":[...]}, ... ] } },
    { "key": "variacao", "label": "...", "dataset": { ..., "threshold": 150 } },
    { "key": "percentual", "label": "...", "dataset": { ... } }
  ],
  "resumo": {
    "vazio": false, "e_r_min": 2500, "e_r_max": 2900,
    "e_r_media": 2777, "pct_estavel": 92,
    "ultimo_estavel": true, "ultimo_percentual": 95
  }
}
```

---

## Fluxo hardware ↔ backend ↔ site (resumo)

```
[ESP32] --POST /data-->        [BACKEND] --grava--> info + info_historico
[ESP32] <--GET /config?id=--   [BACKEND] <--lê----- info (E_b, timer)

[SITE]  --POST /hardware/{id}/config-->  grava E_b/timer (o ESP32 puxa depois)
[SITE]  --GET  /hardware/{id}/info--->   última leitura
[SITE]  --GET  /analytics/{id}/dashboard--> gráficos + resumo
```

O backend é sempre o intermediário. O hardware é **cliente HTTP** e inicia
as duas pontas (envia leitura e busca config); o backend nunca abre conexão
para o hardware — ele guarda a config pendente que o hardware busca sozinho.
