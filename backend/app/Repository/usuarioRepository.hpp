#pragma once

using namespace sql;
using namespace std;

class usuarioRepository {
    private:
        Connection& conn;
    public:
        usuarioRepository() : conn(Database::getInstance().getConnection()) {}

        void Create(usuario& u) {
            PreparedStatement* stmt = conn.prepareStatement("CALL usuario_Controller('Create', NULL, ?, ?, ?)");
            stmt->setString(1, u.get_nome());
            stmt->setString(2, u.get_email());
            stmt->setString(3, u.get_senha());
            stmt->execute();
            delete stmt;
        }

        void Update(usuario& u) {
            PreparedStatement* stmt = conn.prepareStatement("CALL usuario_Controller('Update', ?, ?, ?, ?)");
            stmt->setInt(1, u.get_id());
            stmt->setString(2, u.get_nome());
            stmt->setString(3, u.get_email());
            stmt->setString(4, u.get_senha());
            stmt->execute();
            delete stmt;
        }

        void Delete(int id) {
            PreparedStatement* stmt = conn.prepareStatement("CALL usuario_Controller('Delete', ?, NULL, NULL, NULL)");
            stmt->setInt(1, id);
            stmt->execute();
            delete stmt;
        }

        ResultSet* Index() {
            Statement* stmt = conn.createStatement();
            ResultSet* res = stmt->executeQuery("CALL usuario_Controller('Index', NULL, NULL, NULL, NULL)");
            delete stmt;
            return res;
        }
};
