#pragma once
// ============================================================
//  app/Models/Dispositivo.hpp
//  Entidade 'dispositivos'.
// ============================================================

#include <string>
#include <json/json.h>
#include <drogon/orm/Row.h>

namespace app::models {

class Dispositivo {
public:
    int getId() const { return id_; }
    const std::string& getNome() const { return nome_; }
    const std::string& getTipo() const { return tipo_; }
    int getIdUsuario() const { return idUsuario_; }

    void setId(int v) { id_ = v; }
    void setNome(std::string v) { nome_ = std::move(v); }
    void setTipo(std::string v) { tipo_ = std::move(v); }
    void setIdUsuario(int v) { idUsuario_ = v; }

    Json::Value toJson() const {
        Json::Value j;
        j["id"] = id_;
        j["nome"] = nome_;
        j["tipo"] = tipo_;
        j["id_usuario"] = idUsuario_;
        return j;
    }

    static Dispositivo fromRow(const drogon::orm::Row& row) {
        Dispositivo d;
        d.id_ = row["id"].as<int>();
        d.nome_ = row["nome"].as<std::string>();
        d.tipo_ = row["tipo"].as<std::string>();
        d.idUsuario_ = row["id_usuario"].as<int>();
        return d;
    }

private:
    int id_{0};
    std::string nome_;
    std::string tipo_;
    int idUsuario_{0};
};

} // namespace app::models
