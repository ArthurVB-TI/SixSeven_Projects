#pragma once
// ============================================================
//  app/Models/DispositivoConection.hpp
//  Entidade da relacao N:N 'dispositivos_conections'.
// ============================================================

#include <json/json.h>
#include <drogon/orm/Row.h>

namespace app::models {

class DispositivoConection {
public:
    int getIdConections() const { return idConections_; }
    int getIdDispositivos() const { return idDispositivos_; }
    void setIdConections(int v) { idConections_ = v; }
    void setIdDispositivos(int v) { idDispositivos_ = v; }

    Json::Value toJson() const {
        Json::Value j;
        j["id_conections"] = idConections_;
        j["id_dispositivos"] = idDispositivos_;
        return j;
    }

    static DispositivoConection fromRow(const drogon::orm::Row& row) {
        DispositivoConection dc;
        dc.idConections_ = row["id_conections"].as<int>();
        dc.idDispositivos_ = row["id_dispositivos"].as<int>();
        return dc;
    }

private:
    int idConections_{0};
    int idDispositivos_{0};
};

} // namespace app::models
