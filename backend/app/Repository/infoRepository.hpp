#pragma once

using namespace mysqlx;
using namespace std;

class infoRepository {
    private:
        Session& sess;
    public:
        infoRepository() : sess(Database::getInstance().getConnection()) {}

        void Create(info& i) {
            sess.sql("CALL info_Controller('Create', ?, ?, ?, ?, ?, ?, ?)")
                .bind(i.get_id(), i.get_E_b(), i.get_E_r(), i.get_timer(), i.get_M_er(), i.get_V_mer(), i.get_M_vmer())
                .execute();
        }

        void Update(info& i) {
            sess.sql("CALL info_Controller('Update', ?, ?, ?, ?, ?, ?, ?)")
                .bind(i.get_id(), i.get_E_b(), i.get_E_r(), i.get_timer(), i.get_M_er(), i.get_V_mer(), i.get_M_vmer())
                .execute();
        }

        void Delete(int id) {
            sess.sql("CALL info_Controller('Delete', ?, NULL, NULL, NULL, NULL, NULL, NULL)")
                .bind(id)
                .execute();
        }

        SqlResult Index() {
            return sess.sql("CALL info_Controller('Index', NULL, NULL, NULL, NULL, NULL, NULL, NULL)")
                .execute();
        }
};
