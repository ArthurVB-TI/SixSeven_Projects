#pragma once

using namespace mysqlx;
using namespace std;

class conectionsRepository {
    private:
        Session& sess;
    public:
        conectionsRepository() : sess(Database::getInstance().getConnection()) {}

        void Create(conections& c) {
            sess.sql("CALL conections_Controller('Create', NULL, ?, ?, ?, ?)")
                .bind(c.get_nome(), c.get_canActive(), c.get_isActive(), c.get_id_usuario())
                .execute();
        }

        void Update(conections& c) {
            sess.sql("CALL conections_Controller('Update', ?, ?, ?, ?, ?)")
                .bind(c.get_id(), c.get_nome(), c.get_canActive(), c.get_isActive(), c.get_id_usuario())
                .execute();
        }

        void Delete(int id) {
            sess.sql("CALL conections_Controller('Delete', ?, NULL, NULL, NULL, NULL)")
                .bind(id)
                .execute();
        }

        SqlResult Index() {
            return sess.sql("CALL conections_Controller('Index', NULL, NULL, NULL, NULL, NULL)")
                .execute();
        }
};
