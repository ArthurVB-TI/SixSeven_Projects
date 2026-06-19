#pragma once

using namespace mysqlx;
using namespace std;

class dispositivosConectionsRepository {
    private:
        Session& sess;
    public:
        dispositivosConectionsRepository() : sess(Database::getInstance().getConnection()) {}

        void Create(dispositivos_conections& dc) {
            sess.sql("CALL dispositivos_conections_Controller('Create', ?, ?)")
                .bind(dc.get_id_conections(), dc.get_id_dispositivos())
                .execute();
        }

        void Delete(int id_conections, int id_dispositivos) {
            sess.sql("CALL dispositivos_conections_Controller('Delete', ?, ?)")
                .bind(id_conections, id_dispositivos)
                .execute();
        }

        SqlResult Index() {
            return sess.sql("CALL dispositivos_conections_Controller('Index', NULL, NULL)")
                .execute();
        }
};
