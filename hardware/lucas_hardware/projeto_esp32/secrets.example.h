#ifndef SECRETS_H
#define SECRETS_H

/*
 * EXEMPLO versionavel. Copie para secrets.h (mesma pasta) e preencha
 * com a sua rede. O secrets.h fica fora do git (.gitignore).
 */

// ---------- Wi-Fi ----------
#define WIFI_SSID     "NOME_DO_SEU_WIFI"
#define WIFI_SENHA    "SENHA_DO_SEU_WIFI"

// ---------- Endpoint que recebe os dados ----------
// IP do PC que roda o backend (ipconfig -> Endereco IPv4) + porta 3000.
#define URL_DESTINO   "http://192.168.0.100:3000/dados"

#endif
