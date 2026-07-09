#pragma once
// ============================================================
//  app/Service/DispositivoService.hpp
//  Regras de negocio dos dispositivos e da relacao N:N.
//  Posse: todo acesso e filtrado/validado pelo id_usuario do
//  token (mesmo criterio do ConectionService::requireOwned).
// ============================================================

#include <set>
#include <json/json.h>
#include "../Repository/DispositivoRepository.hpp"
#include "../Repository/DispositivoConectionRepository.hpp"
#include "../Exceptions/ApiException.hpp"
#include "ConectionService.hpp"

namespace app::service {

class DispositivoService {
public:
    Json::Value list(int idUsuario) {
        Json::Value arr(Json::arrayValue);
        for (auto& d : owned(idUsuario)) arr.append(d.toJson());
        return arr;
    }

    int create(const std::string& nome, const std::string& tipo, int idUsuario) {
        models::Dispositivo d;
        d.setNome(nome);
        d.setTipo(tipo);
        d.setIdUsuario(idUsuario);
        int id = repo_.create(d);
        if (id == 0) throw exceptions::ConflictException("Dispositivo ja existe ou dados invalidos");
        return id;
    }

    void remove(int id, int idUsuario) {
        requireOwned(id, idUsuario);
        repo_.remove(id);
    }

    void link(int idConection, int idDispositivo, int idUsuario) {
        conections_.requireOwned(idConection, idUsuario);
        requireOwned(idDispositivo, idUsuario);
        rel_.create(idConection, idDispositivo);
    }
    void unlink(int idConection, int idDispositivo, int idUsuario) {
        conections_.requireOwned(idConection, idUsuario);
        requireOwned(idDispositivo, idUsuario);
        rel_.remove(idConection, idDispositivo);
    }
    Json::Value links(int idUsuario) {
        std::set<int> meus;
        for (auto& d : owned(idUsuario)) meus.insert(d.getId());
        Json::Value arr(Json::arrayValue);
        for (auto& x : rel_.index())
            if (meus.count(x.getIdDispositivos()) > 0) arr.append(x.toJson());
        return arr;
    }

private:
    std::vector<models::Dispositivo> owned(int idUsuario) {
        std::vector<models::Dispositivo> out;
        for (auto& d : repo_.index())
            if (d.getIdUsuario() == idUsuario) out.push_back(d);
        return out;
    }

    // 404 (e nao 403) para nao revelar que o id existe para outro dono.
    models::Dispositivo requireOwned(int id, int idUsuario) {
        for (auto& d : repo_.index()) {
            if (d.getId() == id) {
                if (d.getIdUsuario() != idUsuario)
                    throw exceptions::NotFoundException("Dispositivo nao encontrado");
                return d;
            }
        }
        throw exceptions::NotFoundException("Dispositivo nao encontrado");
    }

    repository::DispositivoRepository repo_;
    repository::DispositivoConectionRepository rel_;
    service::ConectionService conections_;
};

} // namespace app::service
