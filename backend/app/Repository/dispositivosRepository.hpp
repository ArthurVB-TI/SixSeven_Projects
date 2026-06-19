#pragma once

using namespace mysqlx;
using namespace std;

class dispositivosRepository {
    private:
        Session& sess;
    public:
        dispositivosRepository() : sess(Database::getInstance().getConnection()) {}

        void Create(dispositivos& d) {
            sess.sql("CALL dispositivos_Controller('Create', NULL, ?, ?, ?)")
                .bind(d.get_nome(), d.get_tipo(), d.get_id_usuario())
                .execute();
        }

        void Update(dispositivos& d) {
            sess.sql("CALL dispositivos_Controller('Update', ?, ?, ?, ?)")
                .bind(d.get_id(), d.get_nome(), d.get_tipo(), d.get_id_usuario())
                .execute();
        }

        void Delete(int id) {
            sess.sql("CALL dispositivos_Controller('Delete', ?, NULL, NULL, NULL)")
                .bind(id)
                .execute();
        }

        SqlResult Index() {
            return sess.sql("CALL dispositivos_Controller('Index', NULL, NULL, NULL, NULL)")
                .execute();
        }
};
