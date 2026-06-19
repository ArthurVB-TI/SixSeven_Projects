#pragma once

using namespace sql;
using namespace std;

class dispositivosConectionsRepository {
    private:
        Connection& conn;
    public:
        dispositivosConectionsRepository() : conn(Database::getInstance().getConnection()) {}

        void Create(dispositivos_conections& dc) {
            PreparedStatement* stmt = conn.prepareStatement("CALL dispositivos_conections_Controller('Create', ?, ?)");
            stmt->setInt(1, dc.get_id_conections());
            stmt->setInt(2, dc.get_id_dispositivos());
            stmt->execute();
            delete stmt;
        }

        void Delete(int id_conections, int id_dispositivos) {
            PreparedStatement* stmt = conn.prepareStatement("CALL dispositivos_conections_Controller('Delete', ?, ?)");
            stmt->setInt(1, id_conections);
            stmt->setInt(2, id_dispositivos);
            stmt->execute();
            delete stmt;
        }

        ResultSet* Index() {
            Statement* stmt = conn.createStatement();
            ResultSet* res = stmt->executeQuery("CALL dispositivos_conections_Controller('Index', NULL, NULL)");
            delete stmt;
            return res;
        }
};
