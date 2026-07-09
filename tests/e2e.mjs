// ============================================================
//  tests/e2e.mjs — Teste de ponta a ponta do projeto inteiro
//
//  Percorre o mesmo caminho que o usuário + hardware percorrem:
//   1. saúde do backend (/health)
//   2. registro e login (/auth/*)
//   3. criação de conexão (hardware lógico) e dispositivo
//   4. site grava config do hardware (E_b/timer)
//   5. ESP32 SIMULADO puxa a config e envia leituras (POST /data)
//   6. site lê última leitura (/hardware/{id}/info)
//   7. site monta gráficos (/analytics/{id}/dashboard)
//  As asserções comparam o que o frontend exibiria com o que o
//  hardware de fato enviou (mesmos dados das tabelas info /
//  info_historico do banco).
//
//  Uso: node tests/e2e.mjs [--host localhost] [--port 3000]
//                          [--hw-host H] [--hw-port P]
//
//  --port aponta para a API consumida pelo site. Rodar com
//  --port 3001 (proxy do Next) valida também os rewrites que o
//  browser usa. O hardware NÃO passa pelo proxy: --hw-port deixa
//  o ESP32 simulado falando direto com o backend (default: mesma
//  porta da API).
// ============================================================

import { Esp32Simulator } from "./hardware-sim.mjs";

function arg(name, fallback) {
  const index = process.argv.indexOf(`--${name}`);
  return index >= 0 ? process.argv[index + 1] : fallback;
}

const HOST = arg("host", "localhost");
const PORT = Number(arg("port", 3000));
const HW_HOST = arg("hw-host", HOST);
const HW_PORT = Number(arg("hw-port", PORT));
const BASE = `http://${HOST}:${PORT}`;

const PUSHES = 15;          // leituras enviadas pelo hardware simulado
const CONFIG_E_B = 2600;    // energia base que o site vai configurar
const CONFIG_TIMER = 2000;  // timer que o site vai configurar (ms)

let accessToken = null;
const results = [];

function check(name, condition, detail = "") {
  results.push({ name, ok: !!condition, detail });
  const mark = condition ? "PASS" : "FAIL";
  console.log(`  [${mark}] ${name}${detail ? ` — ${detail}` : ""}`);
  return !!condition;
}

async function api(path, { method = "GET", body, raw = false } = {}) {
  const response = await fetch(`${BASE}${path}`, {
    method,
    headers: {
      "Content-Type": "application/json",
      ...(accessToken ? { Authorization: `Bearer ${accessToken}` } : {}),
    },
    body: body !== undefined ? JSON.stringify(body) : undefined,
  });
  const json = await response.json().catch(() => null);
  if (raw) return { status: response.status, json };
  if (!json?.ok) {
    throw new Error(`${method} ${path} -> HTTP ${response.status}: ${json?.error ?? "sem corpo"}`);
  }
  return json.data;
}

console.log(`\n=== Teste E2E SixSeven_Projects — backend ${BASE} ===\n`);

// ---- 1. Saúde ----
console.log("[1/7] Saúde do backend");
const health = await api("/health");
check("GET /health responde up", health.status === "up", `env=${health.env}`);

// ---- 2. Registro + login ----
console.log("\n[2/7] Autenticação");
const stamp = Date.now();
const email = `e2e.${stamp}@sixseven.test`;
const senha = "SenhaForte!123";
const registered = await api("/auth/register", { method: "POST", body: { nome: "Usuário E2E", email, senha } });
check("POST /auth/register cria usuário", Number.isInteger(registered.id), `id=${registered.id} email=${email}`);

const login = await api("/auth/login", { method: "POST", body: { email, senha } });
accessToken = login.access_token;
check("POST /auth/login devolve access_token", !!accessToken && login.usuario?.email === email);

const me = await api("/auth/me");
check("GET /auth/me identifica o usuário", me.email === email);

// ---- 3. Conexão + dispositivo ----
console.log("\n[3/7] Conexão (hardware) e dispositivo");
const { id: conexaoId } = await api("/conections", {
  method: "POST",
  body: { nome: `Bancada E2E ${stamp}`, can_active: true },
});
check("POST /conections cria conexão", Number.isInteger(conexaoId), `id=${conexaoId}`);

await api(`/conections/${conexaoId}`, {
  method: "PUT",
  body: { nome: `Bancada E2E ${stamp}`, can_active: true, is_active: true },
});
const conexao = await api(`/conections/${conexaoId}`);
check("PUT /conections liga o hardware (is_active)", conexao.is_active === true || conexao.is_active === 1);

const { id: dispositivoId } = await api("/dispositivos", {
  method: "POST",
  body: { nome: "Sensor de energia E2E", tipo: "sensor" },
});
await api("/dispositivos/link", { method: "POST", body: { id_conection: conexaoId, id_dispositivo: dispositivoId } });
const links = await api("/dispositivos/links");
check(
  "POST /dispositivos + link associa à conexão",
  links.some((l) => l.id_conections === conexaoId && l.id_dispositivos === dispositivoId),
);

// ---- 4. Site grava config que o hardware vai puxar ----
console.log("\n[4/7] Config do hardware via site");
await api(`/hardware/${conexaoId}/config`, { method: "POST", body: { E_b: CONFIG_E_B, timer: CONFIG_TIMER } });
check("POST /hardware/{id}/config grava E_b/timer", true, `E_b=${CONFIG_E_B} timer=${CONFIG_TIMER}`);

// ---- 5. Hardware simulado: pull da config + push das leituras ----
console.log(`\n[5/7] ESP32 simulado (${PUSHES} envios)`);
const sim = new Esp32Simulator({ id: conexaoId, host: HW_HOST, port: HW_PORT });
const { sent, sentOk } = await sim.run({ pushes: PUSHES, speed: 50 });
check(`POST /data aceita todas as leituras`, sentOk === sent, `${sentOk}/${sent}`);
check(
  "GET /config aplicou E_b/timer do site no firmware",
  sim.E_b === CONFIG_E_B && sim.timer === CONFIG_TIMER,
  `E_b=${sim.E_b} timer=${sim.timer}`,
);

// ---- 6. Última leitura pelo site (mesma linha da tabela `info`) ----
console.log("\n[6/7] Última leitura (/hardware/{id}/info)");
const info = await api(`/hardware/${conexaoId}/info`);
const lastPush = sim.toJson();
check(
  "info espelha o último push do hardware",
  info.E_r === lastPush.E_r && info.M_er === lastPush.M_er &&
    info.V_mer === lastPush.V_mer && info.M_vmer === lastPush.M_vmer &&
    info.E_b === lastPush.E_b,
  `E_r=${info.E_r} M_er=${info.M_er} V_mer=${info.V_mer}`,
);
check(
  "info traz derivadas p/ o frontend (estavel, percentual)",
  typeof info.estavel === "boolean" && Number.isFinite(info.percentual_energia),
  `estavel=${info.estavel} percentual=${info.percentual_energia}%`,
);

// ---- 7. Dashboard de gráficos (mesmas linhas da `info_historico`) ----
console.log("\n[7/7] Analytics (/analytics/{id}/dashboard)");
const dashboard = await api(`/analytics/${conexaoId}/dashboard?limite=100`);
check("dashboard registra todos os pontos enviados", dashboard.pontos === PUSHES, `pontos=${dashboard.pontos}`);
check(
  "dashboard traz os 3 gráficos",
  Array.isArray(dashboard.charts) && dashboard.charts.length >= 3 &&
    dashboard.charts.every((c) => c.dataset?.labels?.length === PUSHES),
  dashboard.charts?.map((c) => c.key).join(", "),
);
const serieEr = dashboard.charts
  ?.find((c) => c.key === "energia_vs_media")
  ?.dataset.series.find((s) => s.name.startsWith("E_r"));
check(
  "série E_r do gráfico termina no último valor enviado",
  serieEr && serieEr.data[serieEr.data.length - 1] === lastPush.E_r,
  `último ponto=${serieEr?.data[serieEr.data.length - 1]}`,
);
check(
  "resumo estatístico coerente",
  dashboard.resumo && dashboard.resumo.vazio === false &&
    dashboard.resumo.e_r_min <= lastPush.E_r && lastPush.E_r <= dashboard.resumo.e_r_max,
  `min=${dashboard.resumo?.e_r_min} max=${dashboard.resumo?.e_r_max} média=${dashboard.resumo?.e_r_media}`,
);

// Listagens que alimentam as páginas do frontend
const conexoes = await api("/conections");
const dispositivos = await api("/dispositivos");
check(
  "listagens do frontend incluem os registros criados",
  conexoes.some((c) => c.id === conexaoId) && dispositivos.some((d) => d.id === dispositivoId),
  `conexões=${conexoes.length} dispositivos=${dispositivos.length}`,
);

// ---- Resumo ----
const failed = results.filter((r) => !r.ok);
console.log(`\n=== Resultado: ${results.length - failed.length}/${results.length} asserções passaram ===`);
if (failed.length) {
  console.log("Falhas:");
  for (const f of failed) console.log(`  - ${f.name}${f.detail ? ` (${f.detail})` : ""}`);
  process.exit(1);
}
console.log(`Conexão de teste: #${conexaoId} (usuário ${email} / senha ${senha})`);
process.exit(0);
