#pragma once

using namespace sql;
using namespace std;

class infoRepository {
    private:
        Connection& conn;
    public:
        infoRepository() : conn(Database::getInstance().getConnection()) {}

        void Create(info& i) {
            PreparedStatement* stmt = conn.prepareStatement("CALL info_Controller('Create', ?, ?, ?, ?, ?, ?, ?)");
            stmt->setInt(1, i.get_id());
            stmt->setInt(2, i.get_E_b());
            stmt->setInt(3, i.get_E_r());
            stmt->setInt(4, i.get_timer());
            stmt->setInt(5, i.get_M_er());
            stmt->setInt(6, i.get_V_mer());
            stmt->setInt(7, i.get_M_vmer());
            stmt->execute();
            delete stmt;
        }

        void Update(info& i) {
            PreparedStatement* stmt = conn.prepareStatement("CALL info_Controller('Update', ?, ?, ?, ?, ?, ?, ?)");
            stmt->setInt(1, i.get_id());
            stmt->setInt(2, i.get_E_b());
            stmt->setInt(3, i.get_E_r());
            stmt->setInt(4, i.get_timer());
            stmt->setInt(5, i.get_M_er());
            stmt->setInt(6, i.get_V_mer());
            stmt->setInt(7, i.get_M_vmer());
            stmt->execute();
            delete stmt;
        }

        void Delete(int id) {
            PreparedStatement* stmt = conn.prepareStatement("CALL info_Controller('Delete', ?, NULL, NULL, NULL, NULL, NULL, NULL)");
            stmt->setInt(1, id);
            stmt->execute();
            delete stmt;
        }

        ResultSet* Index() {
            Statement* stmt = conn.createStatement();
            ResultSet* res = stmt->executeQuery("CALL info_Controller('Index', NULL, NULL, NULL, NULL, NULL, NULL, NULL)");
            delete stmt;
            return res;
        }
};
