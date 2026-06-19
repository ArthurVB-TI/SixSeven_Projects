#pragma once

using namespace sql;

class Database {
    private:
        mysql::MySQL_Driver* driver;
        Connection* conn;

        Database() {
            driver = mysql::get_mysql_driver_instance();
            conn = driver->connect("tcp://localhost:3306", "seu_usuario", "sua_senha");
            conn->setSchema("seu_banco");
        }

        ~Database() {
            delete conn;
        }

    public:
        static Database& getInstance() {
            static Database instance;
            return instance;
        }

        Connection& getConnection() {
            return *conn;
        }

        Database(const Database&) = delete;
        Database& operator=(const Database&) = delete;
};
