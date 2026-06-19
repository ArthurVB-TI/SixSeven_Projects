#include <mysqlx/xdevapi.h>
#include <iostream>

using namespace mysqlx;

int main() {
    try {
        // Conecta ao banco de dados (Host, Porta, Usuário, Senha)
        Session sess("localhost", 33060, "seu_usuario", "sua_senha");

        // Seleciona o schema (banco de dados)
        Schema db = sess.getSchema("seu_banco");

        // Exemplo de execução de query (SQL tradicional)
        RowResult result = sess.sql("SELECT VERSION()").execute();
        
        Row row = result.fetchOne();
        std.cout << "Versao do MySQL: " << row[0] << std.endl;

        sess.close();

    } catch (const Error &err) {
        std.cerr << "Erro no MySQL: " << err.what() << std.endl;
        return 1;
    }
    return 0;
}
