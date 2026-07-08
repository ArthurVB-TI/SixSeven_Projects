#pragma once
// ============================================================
//  app/Service/DispositivoService.hpp
//  Regras de negocio dos dispositivos e da relacao N:N.
// ============================================================

#include <json/json.h>
#include "../Repository/DispositivoRepository.hpp"
#include "../Repository/DispositivoConectionRepository.hpp"
#include "../Exceptions/ApiException.hpp"

namespace app::service {

class DispositivoService {
public:
    Json::Value list() {
        auto lista = repo_.index();
        Json::Value arr(Json::arrayValue);
        for (auto& d : lista) arr.append(d.toJson());
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

    void remove(int id) { repo_.remove(id); }

    void link(int idConection, int idDispositivo) {
        rel_.create(idConection, idDispositivo);
    }
    void unlink(int idConection, int idDispositivo) {
        rel_.remove(idConection, idDispositivo);
    }
    Json::Value links() {
        auto lista = rel_.index();
        Json::Value arr(Json::arrayValue);
        for (auto& x : lista) arr.append(x.toJson());
        return arr;
    }

private:
    repository::DispositivoRepository repo_;
    repository::DispositivoConectionRepository rel_;
};

} // namespace app::service
