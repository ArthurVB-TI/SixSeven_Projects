#pragma once
// ============================================================
//  app/Models/InfoHistorico.hpp
//  Ponto da serie temporal 'info_historico' (para graficos).
// ============================================================

#include <json/json.h>
#include <drogon/orm/Row.h>
#include <string>

namespace app::models {

class InfoHistorico {
public:
    Json::Value toJson() const {
        Json::Value j;
        j["id"] = static_cast<Json::Int64>(id_);
        j["id_conection"] = idConection_;
        j["E_b"] = E_b_;
        j["E_r"] = E_r_;
        j["timer"] = timer_;
        j["M_er"] = M_er_;
        j["V_mer"] = V_mer_;
        j["M_vmer"] = M_vmer_;
        j["criado_em"] = criadoEm_;
        return j;
    }

    static InfoHistorico fromRow(const drogon::orm::Row& row) {
        InfoHistorico h;
        h.id_ = row["id"].as<int64_t>();
        h.idConection_ = row["id_conection"].as<int>();
        h.E_b_ = row["E_b"].as<int>();
        h.E_r_ = row["E_r"].as<int>();
        h.timer_ = row["timer"].as<int>();
        h.M_er_ = row["M_er"].as<int>();
        h.V_mer_ = row["V_mer"].as<int>();
        h.M_vmer_ = row["M_vmer"].as<int>();
        h.criadoEm_ = row["criado_em"].as<std::string>();
        return h;
    }

private:
    int64_t id_{0};
    int idConection_{0};
    int E_b_{0}, E_r_{0}, timer_{0}, M_er_{0}, V_mer_{0}, M_vmer_{0};
    std::string criadoEm_;
};

} // namespace app::models
