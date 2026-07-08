#pragma once
// ============================================================
//  app/Models/Conection.hpp
//  Entidade 'conections' (1 conexao = 1 hardware).
// ============================================================

#include <string>
#include <json/json.h>
#include <drogon/orm/Row.h>

namespace app::models {

class Conection {
public:
    int getId() const { return id_; }
    const std::string& getNome() const { return nome_; }
    bool getCanActive() const { return canActive_; }
    bool getIsActive() const { return isActive_; }
    int getIdUsuario() const { return idUsuario_; }

    void setId(int v) { id_ = v; }
    void setNome(std::string v) { nome_ = std::move(v); }
    void setCanActive(bool v) { canActive_ = v; }
    void setIsActive(bool v) { isActive_ = v; }
    void setIdUsuario(int v) { idUsuario_ = v; }

    Json::Value toJson() const {
        Json::Value j;
        j["id"] = id_;
        j["nome"] = nome_;
        j["can_active"] = canActive_;
        j["is_active"] = isActive_;
        j["id_usuario"] = idUsuario_;
        return j;
    }

    static Conection fromRow(const drogon::orm::Row& row) {
        Conection c;
        c.id_ = row["id"].as<int>();
        c.nome_ = row["nome"].as<std::string>();
        c.canActive_ = row["can_Active"].as<int>() != 0;
        c.isActive_ = row["is_Active"].as<int>() != 0;
        c.idUsuario_ = row["id_usuario"].as<int>();
        return c;
    }

private:
    int id_{0};
    std::string nome_;
    bool canActive_{false};
    bool isActive_{false};
    int idUsuario_{0};
};

} // namespace app::models
