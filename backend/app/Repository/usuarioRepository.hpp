#pragma once

using namespace mysqlx;
using namespace std;

class usuarioRepository {
    private:
        Session& sess;
    public:
        usuarioRepository() : sess(Database::getInstance().getConnection()) {}

        void Create(usuario& u) {
            sess.sql("CALL usuario_Controller('Create', NULL, ?, ?, ?)")
                .bind(u.get_nome(), u.get_email(), u.get_senha())
                .execute();
        }

        void Update(usuario& u) {
            sess.sql("CALL usuario_Controller('Update', ?, ?, ?, ?)")
                .bind(u.get_id(), u.get_nome(), u.get_email(), u.get_senha())
                .execute();
        }

        void Delete(int id) {
            sess.sql("CALL usuario_Controller('Delete', ?, NULL, NULL, NULL)")
                .bind(id)
                .execute();
        }

        SqlResult Index() {
            return sess.sql("CALL usuario_Controller('Index', NULL, NULL, NULL, NULL)")
                .execute();
        }
};
