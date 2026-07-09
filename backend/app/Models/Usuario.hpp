#pragma once
// ============================================================
//  app/Models/Usuario.hpp
//  Entidade de dominio 'usuario'. POJO com getters/setters e
//  serializacao JSON. A senha (hash) nunca vai para o JSON
//  publico (toJson omite; toJsonInternal inclui para uso interno).
// ============================================================

#include <string>
#include <json/json.h>
#include <drogon/orm/Row.h>

namespace app::models {

class Usuario {
public:
    int getId() const { return id_; }
    const std::string& getNome() const { return nome_; }
    const std::string& getEmail() const { return email_; }
    const std::string& getSenha() const { return senha_; }

    void setId(int v) { id_ = v; }
    void setNome(std::string v) { nome_ = std::move(v); }
    void setEmail(std::string v) { email_ = std::move(v); }
    void setSenha(std::string v) { senha_ = std::move(v); }

    // JSON publico (sem senha).
    Json::Value toJson() const {
        Json::Value j;
        j["id"] = id_;
        j["nome"] = nome_;
        j["email"] = email_;
        return j;
    }

    static Usuario fromRow(const drogon::orm::Row& row) {
        Usuario u;
        u.id_ = row["id"].as<int>();
        u.nome_ = row["nome"].as<std::string>();
        u.email_ = row["email"].as<std::string>();
        // A view index_usuario nao expoe a coluna 'senha' (de
        // proposito); acessar coluna inexistente lanca RangeError.
        try {
            if (!row["senha"].isNull())
                u.senha_ = row["senha"].as<std::string>();
        } catch (const std::exception&) {
            // linha veio de uma view sem senha — segue sem o hash
        }
        return u;
    }

private:
    int id_{0};
    std::string nome_;
    std::string email_;
    std::string senha_; // hash bcrypt
};

} // namespace app::models
