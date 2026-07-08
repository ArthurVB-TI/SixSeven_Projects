#pragma once
// ============================================================
//  app/Security/PasswordHasher.hpp
//  Hash de senha com bcrypt. Interface pequena e independente
//  (metodos estaticos) para o Service de autenticacao usar.
//
//  Implementacao: usa a libbcrypt (bcrypt.h / crypt_blowfish),
//  incluida em composers/ como dependencia. Ver README.
//
//  Se preferir Argon2, basta trocar a implementacao aqui: o
//  resto do sistema so conhece hash()/verify() (padrao Strategy).
// ============================================================

#include <string>
#include <stdexcept>

extern "C" {
#include <bcrypt/bcrypt.h>   // libbcrypt (vendored) - ver README
}

#include "../Config/AppConfig.hpp"

namespace app::security {

class PasswordHasher {
public:
    // Gera hash bcrypt (inclui salt aleatorio). Retorna string de 60 chars.
    static std::string hash(const std::string& plain) {
        char salt[BCRYPT_HASHSIZE];
        char hashed[BCRYPT_HASHSIZE];

        int workFactor = app::config::AppConfig::get().bcryptCost();
        if (bcrypt_gensalt(workFactor, salt) != 0)
            throw std::runtime_error("Falha ao gerar salt bcrypt");

        if (bcrypt_hashpw(plain.c_str(), salt, hashed) != 0)
            throw std::runtime_error("Falha ao gerar hash bcrypt");

        return std::string(hashed);
    }

    // Confere senha contra hash armazenado. Comparacao constante
    // ja e feita internamente pela libbcrypt.
    static bool verify(const std::string& plain, const std::string& storedHash) {
        if (storedHash.empty()) return false;
        int ret = bcrypt_checkpw(plain.c_str(), storedHash.c_str());
        // bcrypt_checkpw: 0 = match, >0 = no match, -1 = erro
        return ret == 0;
    }
};

} // namespace app::security
