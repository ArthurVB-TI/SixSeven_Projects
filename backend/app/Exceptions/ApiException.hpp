#pragma once
// ============================================================
//  app/Exceptions/ApiException.hpp
//  Hierarquia de excecoes de dominio. Cada uma carrega um
//  HTTP status e uma mensagem segura para o cliente. O
//  middleware/handler central converte em JSON de erro.
// ============================================================

#include <drogon/HttpTypes.h>
#include <stdexcept>
#include <string>

namespace app::exceptions {

class ApiException : public std::runtime_error {
public:
    ApiException(drogon::HttpStatusCode status, std::string message)
        : std::runtime_error(message), status_(status), message_(std::move(message)) {}

    drogon::HttpStatusCode status() const noexcept { return status_; }
    const std::string& message() const noexcept { return message_; }

private:
    drogon::HttpStatusCode status_;
    std::string message_;
};

// 400
class BadRequestException : public ApiException {
public:
    explicit BadRequestException(std::string msg = "Requisicao invalida")
        : ApiException(drogon::k400BadRequest, std::move(msg)) {}
};

// 401
class UnauthorizedException : public ApiException {
public:
    explicit UnauthorizedException(std::string msg = "Nao autenticado")
        : ApiException(drogon::k401Unauthorized, std::move(msg)) {}
};

// 403
class ForbiddenException : public ApiException {
public:
    explicit ForbiddenException(std::string msg = "Acesso negado")
        : ApiException(drogon::k403Forbidden, std::move(msg)) {}
};

// 404
class NotFoundException : public ApiException {
public:
    explicit NotFoundException(std::string msg = "Recurso nao encontrado")
        : ApiException(drogon::k404NotFound, std::move(msg)) {}
};

// 409
class ConflictException : public ApiException {
public:
    explicit ConflictException(std::string msg = "Conflito de dados")
        : ApiException(drogon::k409Conflict, std::move(msg)) {}
};

// 422
class ValidationException : public ApiException {
public:
    explicit ValidationException(std::string msg = "Falha de validacao")
        : ApiException(drogon::k422UnprocessableEntity, std::move(msg)) {}
};

// 500
class InternalException : public ApiException {
public:
    explicit InternalException(std::string msg = "Erro interno")
        : ApiException(drogon::k500InternalServerError, std::move(msg)) {}
};

} // namespace app::exceptions
