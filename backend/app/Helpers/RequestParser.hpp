#pragma once
// ============================================================
//  app/Helpers/RequestParser.hpp
//  Extrai e valida campos do corpo JSON com mensagens claras.
//  Lanca ValidationException quando um campo obrigatorio falta
//  ou tem tipo errado, centralizando o tratamento de entrada.
// ============================================================

#include <drogon/HttpRequest.h>
#include <json/json.h>
#include <string>
#include "../Exceptions/ApiException.hpp"

namespace app::helpers {

class RequestParser {
public:
    explicit RequestParser(const drogon::HttpRequestPtr& req) {
        auto json = req->getJsonObject();
        if (json) {
            body_ = *json;
        } else {
            // corpo ausente ou nao-JSON: trata como objeto vazio,
            // os getters obrigatorios cuidarao de reclamar.
            body_ = Json::Value(Json::objectValue);
        }
    }

    explicit RequestParser(Json::Value body) : body_(std::move(body)) {}

    std::string requireString(const char* key) const {
        assertPresent(key);
        if (!body_[key].isString())
            throw exceptions::ValidationException(std::string("Campo '") + key + "' deve ser texto");
        return body_[key].asString();
    }

    int requireInt(const char* key) const {
        assertPresent(key);
        const auto& v = body_[key];
        if (!v.isInt() && !v.isUInt() && !(v.isNumeric()))
            throw exceptions::ValidationException(std::string("Campo '") + key + "' deve ser numero");
        return v.asInt();
    }

    bool optionalBool(const char* key, bool def) const {
        if (!body_.isMember(key) || body_[key].isNull()) return def;
        return body_[key].asBool();
    }

    int optionalInt(const char* key, int def) const {
        if (!body_.isMember(key) || body_[key].isNull()) return def;
        return body_[key].asInt();
    }

    std::string optionalString(const char* key, const std::string& def) const {
        if (!body_.isMember(key) || body_[key].isNull()) return def;
        return body_[key].asString();
    }

    const Json::Value& raw() const { return body_; }

private:
    void assertPresent(const char* key) const {
        if (!body_.isMember(key) || body_[key].isNull())
            throw exceptions::ValidationException(std::string("Campo '") + key + "' e obrigatorio");
    }

    Json::Value body_;
};

} // namespace app::helpers
