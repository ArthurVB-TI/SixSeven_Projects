#pragma once

using namespace sql;
using namespace std;

class conectionsRepository {
    private:
        Connection& conn;
    public:
        conectionsRepository() : conn(Database::getInstance().getConnection()) {}

        void Create(conections& c) {
            PreparedStatement* stmt = conn.prepareStatement("CALL conections_Controller('Create', NULL, ?, ?, ?, ?)");
            stmt->setString(1, c.get_nome());
            stmt->setBoolean(2, c.get_canActive());
            stmt->setBoolean(3, c.get_isActive());
            stmt->setInt(4, c.get_id_usuario());
            stmt->execute();
            delete stmt;
        }

        void Update(conections& c) {
            PreparedStatement* stmt = conn.prepareStatement("CALL conections_Controller('Update', ?, ?, ?, ?, ?)");
            stmt->setInt(1, c.get_id());
            stmt->setString(2, c.get_nome());
            stmt->setBoolean(3, c.get_canActive());
            stmt->setBoolean(4, c.get_isActive());
            stmt->setInt(5, c.get_id_usuario());
            stmt->execute();
            delete stmt;
        }

        void Delete(int id) {
            PreparedStatement* stmt = conn.prepareStatement("CALL conections_Controller('Delete', ?, NULL, NULL, NULL, NULL)");
            stmt->setInt(1, id);
            stmt->execute();
            delete stmt;
        }

        ResultSet* Index() {
            Statement* stmt = conn.createStatement();
            ResultSet* res = stmt->executeQuery("CALL conections_Controller('Index', NULL, NULL, NULL, NULL, NULL)");
            delete stmt;
            return res;
        }
};
