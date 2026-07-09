// ============================================================
//  tests/hardware-sim.mjs — Simulador do firmware ESP32
//
//  Reproduz fielmente o loop do sketch SixSeven_Projects.ino:
//   * a cada READ_INTERVAL: "lê o potenciômetro" (E_r) e roda o
//     manipulator (média móvel M_er, variação V_mer, média M_vmer,
//     janela WINDOW_SIZE=10 — mesma aritmética inteira do data.hpp);
//   * a cada `timer` ms: pull() GET /config?id=N (aplica E_b/timer
//     vindos do servidor, como o hooks.hpp) e push() POST /data
//     com {id,E_b,E_r,timer,M_er,V_mer,M_vmer}.
//
//  Uso:
//    node tests/hardware-sim.mjs --id 1 [--host localhost] [--port 3000]
//                                [--pushes 12] [--speed 20] [--base 2700]
//
//    --pushes N  para após N envios (0 = roda para sempre)
//    --speed X   acelera o tempo simulado X vezes (1 = tempo real)
//    --base V    nível "real" do potenciômetro simulado (0..4095)
// ============================================================

// ---- constantes do firmware (config.hpp) ----
const READ_INTERVAL = 100;   // ms entre leituras do sensor
const SEND_INTERVAL = 2000;  // timer inicial até o servidor mandar outro
const WINDOW_SIZE = 10;      // janela da média móvel

function parseArgs(argv) {
  const args = { id: 0, host: "localhost", port: 3000, pushes: 12, speed: 20, base: 2700 };
  for (let i = 0; i < argv.length; i += 2) {
    const key = argv[i]?.replace(/^--/, "");
    const value = argv[i + 1];
    if (key in args) args[key] = key === "host" ? value : Number(value);
  }
  return args;
}

// Média móvel com aritmética inteira idêntica ao DataRepository (data.hpp).
class MovingWindow {
  constructor() {
    this.history = new Array(WINDOW_SIZE).fill(0);
    this.count = 0;
    this.sum = 0;
  }
  push(value) {
    const index = this.count % WINDOW_SIZE;
    this.sum -= this.history[index];
    this.history[index] = value;
    this.sum += value;
    if (this.count < WINDOW_SIZE) this.count++;
    const n = Math.min(this.count, WINDOW_SIZE);
    return Math.trunc(this.sum / n);
  }
}

// Potenciômetro simulado: nível base + deriva senoidal lenta + ruído do ADC,
// com um "tranco" ocasional (alguém girou o knob) para gerar instabilidade.
class FakePotentiometer {
  constructor(base) {
    this.base = base;
    this.t = 0;
    this.kick = 0;
  }
  read() {
    this.t++;
    if (this.kick === 0 && Math.random() < 0.01) this.kick = 40; // tranco raro
    let level = this.base + Math.round(180 * Math.sin(this.t / 90));
    if (this.kick > 0) {
      level += Math.round(600 * Math.sin(this.kick / 6));
      this.kick--;
    }
    level += Math.round((Math.random() - 0.5) * 60); // ruído do ADC
    return Math.max(0, Math.min(4095, level));
  }
}

export class Esp32Simulator {
  constructor({ id, host, port, base = 2700 }) {
    this.baseUrl = `http://${host}:${port}`;
    // estado do Data (data.hpp): initData()
    this.id = id;
    this.E_b = 0;
    this.E_r = 0;
    this.timer = SEND_INTERVAL;
    this.M_er = 0;
    this.V_mer = 0;
    this.M_vmer = 0;
    this.received = new MovingWindow();
    this.variation = new MovingWindow();
    this.pot = new FakePotentiometer(base);
  }

  // collector() + manipulator(): uma leitura do sensor
  tick() {
    this.E_r = this.pot.read();
    this.M_er = this.received.push(this.E_r);
    this.V_mer = Math.abs(this.E_r - this.M_er);
    this.M_vmer = this.variation.push(this.V_mer);
  }

  toJson() {
    const { id, E_b, E_r, timer, M_er, V_mer, M_vmer } = this;
    return { id, E_b, E_r, timer, M_er, V_mer, M_vmer };
  }

  // hooks.pull(): GET /config?id=N — aplica E_b e timer do servidor
  async pull() {
    const response = await fetch(`${this.baseUrl}/config?id=${this.id}`);
    if (!response.ok) return false;
    const cfg = await response.json();
    if (Number.isInteger(cfg.id)) this.id = cfg.id;
    if (Number.isInteger(cfg.E_b)) this.E_b = cfg.E_b;
    if (Number.isInteger(cfg.timer)) this.timer = cfg.timer;
    return true;
  }

  // hooks.push(): POST /data com o payload cru do firmware
  async push() {
    const response = await fetch(`${this.baseUrl}/data`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(this.toJson()),
    });
    return response.ok;
  }

  printer(pushOk, pullOk) {
    const flag = pushOk ? "ok" : "FALHOU";
    console.log(
      `id:${this.id} E_b:${this.E_b} E_r:${this.E_r} M_er:${this.M_er}` +
      ` V_mer:${this.V_mer} M_vmer:${this.M_vmer} timer:${this.timer}` +
      ` | pull:${pullOk ? "ok" : "--"} push:${flag}`,
    );
  }

  // loop() do sketch: leituras a cada READ_INTERVAL, envio a cada timer.
  async run({ pushes = 12, speed = 20, onPush = null } = {}) {
    const sleep = (ms) => new Promise((r) => setTimeout(r, Math.max(1, ms / speed)));
    let sent = 0;
    let sinceSend = 0;
    let sentOk = 0;
    while (pushes === 0 || sent < pushes) {
      this.tick();
      sinceSend += READ_INTERVAL;
      await sleep(READ_INTERVAL);
      if (sinceSend >= this.timer) {
        sinceSend = 0;
        const pullOk = await this.pull().catch(() => false);
        const pushOk = await this.push().catch(() => false);
        this.printer(pushOk, pullOk);
        sent++;
        if (pushOk) sentOk++;
        if (onPush) await onPush(this, { pushOk, pullOk, sent });
      }
    }
    return { sent, sentOk };
  }
}

// Execução direta: node tests/hardware-sim.mjs --id 1 ...
const invokedDirectly = process.argv[1] && import.meta.url.endsWith(process.argv[1].replace(/\\/g, "/").split("/").pop());
if (invokedDirectly) {
  const args = parseArgs(process.argv.slice(2));
  if (!args.id) {
    console.error("uso: node tests/hardware-sim.mjs --id <id da conexao> [--host localhost] [--port 3000] [--pushes 12] [--speed 20] [--base 2700]");
    process.exit(1);
  }
  console.log(`[sim] ESP32 simulado -> ${args.host}:${args.port} conexao #${args.id} (${args.pushes || "infinitos"} envios, ${args.speed}x)`);
  const sim = new Esp32Simulator(args);
  const { sent, sentOk } = await sim.run({ pushes: args.pushes, speed: args.speed });
  console.log(`[sim] fim: ${sentOk}/${sent} envios aceitos pelo backend`);
  process.exit(sentOk === sent ? 0 : 1);
}
