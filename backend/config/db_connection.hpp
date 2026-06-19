#pragma once
#include <mysqlx/xdevapi.h>
#include <iostream>

using namespace mysqlx;

class Database {
    private:
        Session* sess;

        Database() {
            sess = new Session("localhost", 33060, "seu_usuario", "sua_senha");
            sess->sql("USE seu_banco").execute();
        }

        ~Database() {
            sess->close();
            delete sess;
        }

    public:
        static Database& getInstance() {
            static Database instance;
            return instance;
        }

        Session& getConnection() {
            return *sess;
        }

        Database(const Database&) = delete;
        Database& operator=(const Database&) = delete;
};
