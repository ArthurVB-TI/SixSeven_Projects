// ============================================================================
//  BACKEND DO PROJETO ESP32  -  C++ + cpp-httplib + MySQL
// ============================================================================
//
//  O que ele faz:
//    1. Sobe um servidor HTTP na porta 3000 (biblioteca httplib.h).
//    2. Recebe o POST com JSON que o ESP32 manda na rota /dados.
//    3. Le os campos do JSON e faz um INSERT na tabela 'leituras' do MySQL.
//    4. Responde {"status":"ok"} pro ESP32 (assim a pontuacao dele sobe).
//
//  Bibliotecas necessarias:
//    - httplib.h  -> servidor HTTP (1 arquivo, na mesma pasta deste .cpp)
//    - mysql.h    -> Connector/C do MySQL (ja instalado com o MySQL Server)
//
//  IMPORTANTE: este arquivo NAO usa uma biblioteca de JSON pra ficar simples
//  de compilar. Em vez disso, ele extrai os numeros do JSON "na mao" com uma
//  funcao auxiliar. Funciona porque o JSON do ESP32 e simples e previsivel.
// ============================================================================

#include "httplib.h"     // servidor HTTP
#include <mysql.h>       // cliente MySQL (Connector/C)
#include <iostream>
#include <string>
#include <cstdlib>

// ---------------------------------------------------------------------------
// CONFIGURACAO DO BANCO  -- ajuste a SENHA para a do seu root
// ---------------------------------------------------------------------------
const char* DB_HOST  = "127.0.0.1";
const char* DB_USER  = "root";
const char* DB_PASS  = "aura++67";  // <<< troque pela senha do root
const char* DB_NAME  = "projeto_esp32";
const unsigned DB_PORT = 3306;

const int PORTA_HTTP = 3000;

// ---------------------------------------------------------------------------
// Extrai o valor de uma chave do JSON de forma simples.
// Ex: do texto {"energia":2500.0,...} e chave "energia" devolve "2500.0".
// Nao e um parser completo de JSON, mas serve para o formato fixo do ESP32.
// ---------------------------------------------------------------------------
std::string extrairValor(const std::string& json, const std::string& chave) {
    std::string busca = "\"" + chave + "\":";
    size_t pos = json.find(busca);
    if (pos == std::string::npos) return "";

    pos += busca.size();
    // pula espacos
    while (pos < json.size() && json[pos] == ' ') pos++;

    // o valor vai ate a proxima virgula ou ate o fecha-chaves
    size_t fim = json.find_first_of(",}", pos);
    if (fim == std::string::npos) fim = json.size();

    std::string valor = json.substr(pos, fim - pos);

    // remove aspas se for texto (ex: "true")
    if (!valor.empty() && valor.front() == '"') valor = valor.substr(1);
    if (!valor.empty() && valor.back() == '"')  valor.pop_back();
    return valor;
}

int main() {
    // -----------------------------------------------------------------------
    // 1) Conecta no MySQL (uma vez, no inicio)
    // -----------------------------------------------------------------------
    MYSQL* conexao = mysql_init(nullptr);
    if (conexao == nullptr) {
        std::cerr << "Erro: mysql_init falhou.\n";
        return 1;
    }

    if (mysql_real_connect(conexao, DB_HOST, DB_USER, DB_PASS, DB_NAME,
                           DB_PORT, nullptr, 0) == nullptr) {
        std::cerr << "Erro ao conectar no MySQL: " << mysql_error(conexao) << "\n";
        mysql_close(conexao);
        return 1;
    }
    std::cout << "[OK] Conectado ao MySQL (banco: " << DB_NAME << ")\n";

    // -----------------------------------------------------------------------
    // 2) Configura o servidor HTTP
    // -----------------------------------------------------------------------
    httplib::Server servidor;

    // rota POST /dados -> recebe o JSON do ESP32 e grava no banco
    servidor.Post("/dados", [&](const httplib::Request& req, httplib::Response& res) {
        const std::string& corpo = req.body;
        std::cout << "\n[RECEBIDO] " << corpo << "\n";

        // extrai cada campo do JSON
        std::string energia   = extrairValor(corpo, "energia");
        std::string som       = extrairValor(corpo, "som");
        std::string ligadoTxt = extrairValor(corpo, "ligado");
        std::string pontuacao = extrairValor(corpo, "pontuacao");
        std::string uptime    = extrairValor(corpo, "uptime_ms");

        // "true"/"false" -> 1/0 para a coluna BOOLEAN
        int ligado = (ligadoTxt == "true") ? 1 : 0;

        // valores vazios viram 0 para nao quebrar o SQL
        if (energia.empty())   energia   = "0";
        if (som.empty())       som       = "0";
        if (pontuacao.empty()) pontuacao = "0";
        if (uptime.empty())    uptime    = "0";

        // monta o comando INSERT
        std::string sql =
            "INSERT INTO leituras (energia, som, ligado, pontuacao, uptime_ms) VALUES ("
            + energia + ", " + som + ", " + std::to_string(ligado) + ", "
            + pontuacao + ", " + uptime + ")";

        // executa no banco
        if (mysql_query(conexao, sql.c_str()) == 0) {
            std::cout << "[BANCO] Linha inserida com sucesso.\n";
            res.set_content("{\"status\":\"ok\"}", "application/json");
        } else {
            std::cerr << "[BANCO] Erro no INSERT: " << mysql_error(conexao) << "\n";
            res.status = 500;
            res.set_content("{\"status\":\"erro\"}", "application/json");
        }
    });

    // rota GET / -> so um teste rapido pra ver no navegador que esta no ar
    servidor.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("Backend ESP32 em C++ rodando! Os dados vao para /dados",
                        "text/plain");
    });

    // -----------------------------------------------------------------------
    // 3) Sobe o servidor
    // -----------------------------------------------------------------------
    std::cout << "============================================================\n";
    std::cout << "  BACKEND ESP32 (C++ + MySQL) rodando!\n";
    std::cout << "  Recebendo dados em: http://0.0.0.0:" << PORTA_HTTP << "/dados\n";
    std::cout << "  Coloque o IP da sua maquina no config.h do ESP32.\n";
    std::cout << "  Ctrl+C para parar.\n";
    std::cout << "============================================================\n";

    // listen em 0.0.0.0 -> aceita conexoes de outras maquinas (o ESP32) na rede
    servidor.listen("0.0.0.0", PORTA_HTTP);

    mysql_close(conexao);
    return 0;
}
