#pragma once
// ============================================================
//  app/Security/Hash.hpp
//  Utilitarios de hash/aleatoriedade baseados em OpenSSL.
//   * sha256Hex: para guardar o refresh token como hash (nunca
//     armazenamos o token cru no banco).
//   * randomHex: gera tokens opacos criptograficamente seguros.
// ============================================================

#include <openssl/sha.h>
#include <openssl/rand.h>
#include <string>
#include <stdexcept>

namespace app::security {

class Hash {
public:
    static std::string sha256Hex(const std::string& input) {
        unsigned char digest[SHA256_DIGEST_LENGTH];
        SHA256(reinterpret_cast<const unsigned char*>(input.data()),
               input.size(), digest);
        return toHex(digest, SHA256_DIGEST_LENGTH);
    }

    // Gera 'nbytes' aleatorios e retorna como hex (2*nbytes chars).
    static std::string randomHex(size_t nbytes = 32) {
        std::string buf(nbytes, '\0');
        if (RAND_bytes(reinterpret_cast<unsigned char*>(&buf[0]),
                       static_cast<int>(nbytes)) != 1)
            throw std::runtime_error("Falha ao gerar bytes aleatorios");
        return toHex(reinterpret_cast<const unsigned char*>(buf.data()), nbytes);
    }

private:
    static std::string toHex(const unsigned char* data, size_t len) {
        static const char* hexchars = "0123456789abcdef";
        std::string out;
        out.reserve(len * 2);
        for (size_t i = 0; i < len; ++i) {
            out.push_back(hexchars[(data[i] >> 4) & 0xF]);
            out.push_back(hexchars[data[i] & 0xF]);
        }
        return out;
    }
};

} // namespace app::security
