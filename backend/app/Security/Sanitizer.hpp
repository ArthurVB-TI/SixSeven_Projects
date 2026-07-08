#pragma once
// ============================================================
//  app/Security/Sanitizer.hpp
//  Funcoes independentes de saneamento/validacao usadas no
//  tratamento de rotas. Nao substituem prepared statements
//  (esses ja protegem contra SQL injection), mas rejeitam
//  entradas obviamente invalidas cedo (defense-in-depth).
// ============================================================

#include <string>
#include <algorithm>
#include <cctype>
#include "../Exceptions/ApiException.hpp"

namespace app::security {

class Sanitizer {
public:
    // Converte um path/query param em inteiro positivo valido.
    static int toPositiveInt(const std::string& raw, const char* fieldName) {
        if (raw.empty() || !std::all_of(raw.begin(), raw.end(), ::isdigit))
            throw exceptions::BadRequestException(
                std::string("Parametro '") + fieldName + "' invalido");
        try {
            long v = std::stol(raw);
            if (v <= 0 || v > 2147483647L)
                throw exceptions::BadRequestException(
                    std::string("Parametro '") + fieldName + "' fora do intervalo");
            return static_cast<int>(v);
        } catch (const std::exception&) {
            throw exceptions::BadRequestException(
                std::string("Parametro '") + fieldName + "' invalido");
        }
    }

    // Remove espacos nas pontas e rejeita string vazia.
    static std::string requireNonEmpty(std::string s, const char* fieldName) {
        trim(s);
        if (s.empty())
            throw exceptions::ValidationException(
                std::string("Campo '") + fieldName + "' nao pode ser vazio");
        return s;
    }

    static bool looksLikeEmail(const std::string& s) {
        auto at = s.find('@');
        if (at == std::string::npos || at == 0) return false;
        auto dot = s.find('.', at);
        return dot != std::string::npos && dot < s.size() - 1;
    }

private:
    static void trim(std::string& s) {
        auto notSpace = [](int c) { return !std::isspace(c); };
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
        s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
    }
};

} // namespace app::security
