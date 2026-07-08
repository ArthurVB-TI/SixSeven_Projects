#pragma once
// ============================================================
//  app/Service/ConectionService.hpp
//  Regras de negocio das conexoes. Garante que um usuario so
//  enxergue/opere suas proprias conexoes (checagem de posse).
// ============================================================

#include <vector>
#include <json/json.h>
#include "../Repository/ConectionRepository.hpp"
#include "../Exceptions/ApiException.hpp"

namespace app::service {

class ConectionService {
public:
    // Pagina inicial: conexoes do usuario logado.
    Json::Value listByUsuario(int idUsuario) {
        auto lista = repo_.indexByUsuario(idUsuario);
        Json::Value arr(Json::arrayValue);
        for (auto& c : lista) arr.append(c.toJson());
        return arr;
    }

    // Detalhe de uma conexao, validando posse.
    Json::Value getOwned(int idConection, int idUsuario) {
        auto c = requireOwned(idConection, idUsuario);
        return c.toJson();
    }

    int create(const std::string& nome, bool canActive, int idUsuario) {
        models::Conection c;
        c.setNome(nome);
        c.setCanActive(canActive);
        c.setIsActive(false);
        c.setIdUsuario(idUsuario);
        int id = repo_.create(c);
        if (id == 0) throw exceptions::ConflictException("Conexao ja existe ou dados invalidos");
        return id;
    }

    void update(int idConection, int idUsuario, const std::string& nome,
                bool canActive, bool isActive) {
        auto atual = requireOwned(idConection, idUsuario);
        models::Conection c;
        c.setId(idConection);
        c.setNome(nome);
        c.setCanActive(canActive);
        c.setIsActive(isActive);
        c.setIdUsuario(idUsuario);
        repo_.update(c);
    }

    void remove(int idConection, int idUsuario) {
        requireOwned(idConection, idUsuario);
        repo_.remove(idConection);
    }

    // Reutilizada por outros services: valida posse e retorna a conexao.
    models::Conection requireOwned(int idConection, int idUsuario) {
        auto opt = repo_.findById(idConection);
        if (!opt.has_value())
            throw exceptions::NotFoundException("Conexao nao encontrada");
        if (opt->getIdUsuario() != idUsuario)
            throw exceptions::ForbiddenException("Esta conexao nao pertence a voce");
        return *opt;
    }

private:
    repository::ConectionRepository repo_;
};

} // namespace app::service
