#pragma once
// ============================================================
//  app/Security/Jwt.hpp
//  JWT HS256 auto-contido (sem libs externas alem do OpenSSL).
//  Usado para o ACCESS TOKEN de curta duracao. O refresh token
//  e opaco (ver Hash.hpp) e guardado hasheado no banco.
//
//  Formato: header.payload.signature (base64url).
//  Claims usadas: sub (id do usuario), exp (expiracao unix),
//                 iat (emissao), email.
// ============================================================

#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <json/json.h>
#include <string>
#include <ctime>
#include <optional>
#include <cstring>

namespace app::security {

class Jwt {
public:
    struct Claims {
        long   sub{0};        // id do usuario
        std::string email;
        long   exp{0};
        long   iat{0};
    };

    // Gera um token assinado.
    static std::string encode(const Claims& c, const std::string& secret) {
        Json::Value header;
        header["alg"] = "HS256";
        header["typ"] = "JWT";

        Json::Value payload;
        payload["sub"]   = static_cast<Json::Int64>(c.sub);
        payload["email"] = c.email;
        payload["iat"]   = static_cast<Json::Int64>(c.iat);
        payload["exp"]   = static_cast<Json::Int64>(c.exp);

        std::string h = b64url(compact(header));
        std::string p = b64url(compact(payload));
        std::string signingInput = h + "." + p;
        std::string sig = b64url(hmacSha256(signingInput, secret));

        return signingInput + "." + sig;
    }

    // Valida assinatura + expiracao. Retorna claims se valido.
    static std::optional<Claims> decode(const std::string& token,
                                        const std::string& secret) {
        auto p1 = token.find('.');
        if (p1 == std::string::npos) return std::nullopt;
        auto p2 = token.find('.', p1 + 1);
        if (p2 == std::string::npos) return std::nullopt;

        std::string h   = token.substr(0, p1);
        std::string pay = token.substr(p1 + 1, p2 - p1 - 1);
        std::string sig = token.substr(p2 + 1);

        std::string signingInput = h + "." + pay;
        std::string expected = b64url(hmacSha256(signingInput, secret));
        if (!constantTimeEquals(expected, sig)) return std::nullopt;

        Json::Value payload;
        if (!parse(b64urlDecode(pay), payload)) return std::nullopt;

        Claims c;
        c.sub   = payload.get("sub", 0).asInt64();
        c.email = payload.get("email", "").asString();
        c.iat   = payload.get("iat", 0).asInt64();
        c.exp   = payload.get("exp", 0).asInt64();

        if (c.exp != 0 && c.exp < static_cast<long>(std::time(nullptr)))
            return std::nullopt; // expirado

        return c;
    }

private:
    static std::string compact(const Json::Value& v) {
        Json::StreamWriterBuilder b;
        b["indentation"] = "";
        return Json::writeString(b, v);
    }

    static bool parse(const std::string& s, Json::Value& out) {
        Json::CharReaderBuilder b;
        std::string errs;
        std::unique_ptr<Json::CharReader> r(b.newCharReader());
        return r->parse(s.data(), s.data() + s.size(), &out, &errs);
    }

    static std::string hmacSha256(const std::string& data, const std::string& key) {
        unsigned char out[EVP_MAX_MD_SIZE];
        unsigned int len = 0;
        HMAC(EVP_sha256(),
             key.data(), static_cast<int>(key.size()),
             reinterpret_cast<const unsigned char*>(data.data()), data.size(),
             out, &len);
        return std::string(reinterpret_cast<char*>(out), len);
    }

    // base64url sem padding
    static std::string b64url(const std::string& in) {
        static const char* tbl =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
        std::string out;
        int val = 0, bits = -6;
        for (unsigned char c : in) {
            val = (val << 8) + c;
            bits += 8;
            while (bits >= 0) {
                out.push_back(tbl[(val >> bits) & 0x3F]);
                bits -= 6;
            }
        }
        if (bits > -6)
            out.push_back(tbl[((val << 8) >> (bits + 8)) & 0x3F]);
        return out;
    }

    static std::string b64urlDecode(const std::string& in) {
        auto idx = [](char c) -> int {
            if (c >= 'A' && c <= 'Z') return c - 'A';
            if (c >= 'a' && c <= 'z') return c - 'a' + 26;
            if (c >= '0' && c <= '9') return c - '0' + 52;
            if (c == '-') return 62;
            if (c == '_') return 63;
            return -1;
        };
        std::string out;
        int val = 0, bits = -8;
        for (char c : in) {
            int d = idx(c);
            if (d < 0) break;
            val = (val << 6) + d;
            bits += 6;
            if (bits >= 0) {
                out.push_back(static_cast<char>((val >> bits) & 0xFF));
                bits -= 8;
            }
        }
        return out;
    }

    static bool constantTimeEquals(const std::string& a, const std::string& b) {
        if (a.size() != b.size()) return false;
        unsigned char diff = 0;
        for (size_t i = 0; i < a.size(); ++i)
            diff |= static_cast<unsigned char>(a[i] ^ b[i]);
        return diff == 0;
    }
};

} // namespace app::security
