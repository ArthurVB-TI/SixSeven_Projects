#pragma once
// ============================================================
//  app/Service/AuthService.hpp
//  Regras de autenticacao/sessao. Orquestra UsuarioRepository +
//  RefreshTokenRepository + Security (hash/JWT).
//
//  Estrategia de sessao (manter logado):
//   * ACCESS TOKEN  = JWT curto (config: access_token_ttl).
//   * REFRESH TOKEN = string opaca longa; guardamos so o SHA-256
//     no banco. Renova o access sem novo login. Rotacionado a
//     cada uso (o antigo e revogado).
// ============================================================

#include <string>
#include <ctime>
#include <optional>
#include "../Repository/UsuarioRepository.hpp"
#include "../Repository/RefreshTokenRepository.hpp"
#include "../Security/PasswordHasher.hpp"
#include "../Security/Jwt.hpp"
#include "../Security/Hash.hpp"
#include "../Security/Sanitizer.hpp"
#include "../Config/AppConfig.hpp"
#include "../Exceptions/ApiException.hpp"
#include "../Models/Usuario.hpp"

namespace app::service {

struct AuthTokens {
    std::string accessToken;
    std::string refreshToken;   // valor cru, entregue ao cliente 1x
    int accessExpiresIn{0};
    models::Usuario usuario;
};

class AuthService {
public:
    // ---------------- Registro ----------------
    models::Usuario register_(const std::string& nome,
                              const std::string& email,
                              const std::string& senha) {
        auto cleanNome  = security::Sanitizer::requireNonEmpty(nome, "nome");
        auto cleanEmail = security::Sanitizer::requireNonEmpty(email, "email");
        if (!security::Sanitizer::looksLikeEmail(cleanEmail))
            throw exceptions::ValidationException("E-mail invalido");
        if (senha.size() < 6)
            throw exceptions::ValidationException("Senha deve ter ao menos 6 caracteres");

        if (usuarioRepo_.findByEmail(cleanEmail).has_value())
            throw exceptions::ConflictException("E-mail ja cadastrado");

        std::string hash = security::PasswordHasher::hash(senha);
        int id = usuarioRepo_.create(cleanNome, cleanEmail, hash);
        if (id == 0)
            throw exceptions::ConflictException("Nao foi possivel criar o usuario");

        models::Usuario u;
        u.setId(id);
        u.setNome(cleanNome);
        u.setEmail(cleanEmail);
        return u;
    }

    // ---------------- Login ----------------
    AuthTokens login(const std::string& email, const std::string& senha) {
        auto userOpt = usuarioRepo_.findByEmail(email);
        // Mensagem generica: nao revela se o e-mail existe.
        if (!userOpt.has_value())
            throw exceptions::UnauthorizedException("Credenciais invalidas");

        if (!security::PasswordHasher::verify(senha, userOpt->getSenha()))
            throw exceptions::UnauthorizedException("Credenciais invalidas");

        return issueTokens(*userOpt);
    }

    // ---------------- Refresh (rotacao) ----------------
    AuthTokens refresh(const std::string& rawRefreshToken) {
        if (rawRefreshToken.empty())
            throw exceptions::UnauthorizedException("Refresh token ausente");

        std::string hash = security::Hash::sha256Hex(rawRefreshToken);
        auto rowOpt = tokenRepo_.findValid(hash);
        if (!rowOpt.has_value())
            throw exceptions::UnauthorizedException("Sessao expirada, faca login novamente");

        // Rotaciona: revoga o antigo e emite novo par.
        tokenRepo_.revoke(hash);

        auto users = usuarioRepo_.index(); // index nao traz senha; buscamos o dono
        models::Usuario owner;
        bool found = false;
        for (auto& u : users) {
            if (u.getId() == rowOpt->idUsuario) { owner = u; found = true; break; }
        }
        if (!found)
            throw exceptions::UnauthorizedException("Usuario da sessao nao existe mais");

        return issueTokens(owner);
    }

    // ---------------- Logout ----------------
    void logout(const std::string& rawRefreshToken) {
        if (rawRefreshToken.empty()) return;
        std::string hash = security::Hash::sha256Hex(rawRefreshToken);
        tokenRepo_.revoke(hash);
    }

    void logoutAll(int idUsuario) {
        tokenRepo_.revokeAll(idUsuario);
    }

private:
    AuthTokens issueTokens(const models::Usuario& user) {
        const auto& cfg = config::AppConfig::get();
        long now = static_cast<long>(std::time(nullptr));

        security::Jwt::Claims claims;
        claims.sub = user.getId();
        claims.email = user.getEmail();
        claims.iat = now;
        claims.exp = now + cfg.accessTokenTtl();

        AuthTokens out;
        out.accessToken = security::Jwt::encode(claims, cfg.jwtSecret());
        out.accessExpiresIn = cfg.accessTokenTtl();

        // Refresh opaco + persistencia do hash.
        out.refreshToken = security::Hash::randomHex(32);
        std::string refreshHash = security::Hash::sha256Hex(out.refreshToken);
        tokenRepo_.create(user.getId(), refreshHash,
                          formatExpiry(now + cfg.refreshTokenTtl()));

        out.usuario = user;
        return out;
    }

    static std::string formatExpiry(long unixTime) {
        std::time_t t = static_cast<std::time_t>(unixTime);
        std::tm tm{};
#if defined(_WIN32)
        gmtime_s(&tm, &t);
#else
        gmtime_r(&t, &tm);
#endif
        char buf[20];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
        return std::string(buf);
    }

    repository::UsuarioRepository usuarioRepo_;
    repository::RefreshTokenRepository tokenRepo_;
};

} // namespace app::service
