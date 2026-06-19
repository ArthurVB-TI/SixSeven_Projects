#pragma once

using namespace sql;
using namespace std;

class dispositivosRepository {
    private:
        Connection& conn;
    public:
        dispositivosRepository() : conn(Database::getInstance().getConnection()) {}

        void Create(dispositivos& d) {
            PreparedStatement* stmt = conn.prepareStatement("CALL dispositivos_Controller('Create', NULL, ?, ?, ?)");
            stmt->setString(1, d.get_nome());
            stmt->setString(2, d.get_tipo());
            stmt->setInt(3, d.get_id_usuario());
            stmt->execute();
            delete stmt;
        }

        void Update(dispositivos& d) {
            PreparedStatement* stmt = conn.prepareStatement("CALL dispositivos_Controller('Update', ?, ?, ?, ?)");
            stmt->setInt(1, d.get_id());
            stmt->setString(2, d.get_nome());
            stmt->setString(3, d.get_tipo());
            stmt->setInt(4, d.get_id_usuario());
            stmt->execute();
            delete stmt;
        }

        void Delete(int id) {
            PreparedStatement* stmt = conn.prepareStatement("CALL dispositivos_Controller('Delete', ?, NULL, NULL, NULL)");
            stmt->setInt(1, id);
            stmt->execute();
            delete stmt;
        }

        ResultSet* Index() {
            Statement* stmt = conn.createStatement();
            ResultSet* res = stmt->executeQuery("CALL dispositivos_Controller('Index', NULL, NULL, NULL, NULL)");
            delete stmt;
            return res;
        }
};
