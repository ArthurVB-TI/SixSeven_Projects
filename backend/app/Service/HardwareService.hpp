#pragma once
// ============================================================
//  app/Service/HardwareService.hpp
//  A PONTE entre o site e o hardware. Todo trafego passa aqui.
//
//  Fluxos (contrato IDENTICO ao firmware, nao alterar shapes):
//   * ingestPush(json)   <- hardware POST /data
//        payload: {id,E_b,E_r,timer,M_er,V_mer,M_vmer}
//        grava leitura (info) + ponto no historico.
//   * buildConfig(id)    -> hardware GET /config?id=
//        retorna EXATAMENTE {id,E_b,timer} que o firmware le.
//
//   Lado site (autenticado, com posse validada no controller):
//   * currentInfo(id)    -> ultima leitura detalhada + derivadas.
//   * setConfig(id,E_b,timer) -> grava config que o hardware puxa.
//
//  Observacao de arquitetura: o hardware e cliente HTTP e inicia
//  as duas pontas (push e pull). O backend nunca abre conexao
//  para o hardware; ele mantem a config pendente que o hardware
//  busca. Assim o fluxo do enunciado (front->back->hardware e
//  hardware->back) se realiza com o back como intermediario.
// ============================================================

#include <json/json.h>
#include "../Repository/InfoRepository.hpp"
#include "../Models/Info.hpp"
#include "../Exceptions/ApiException.hpp"
#include "../Helpers/RequestParser.hpp"

namespace app::service {

class HardwareService {
public:
    // ---- Hardware -> Back (POST /data) ----
    void ingestPush(const Json::Value& body) {
        helpers::RequestParser p(body);
        models::Info info;
        info.setId(p.requireInt("id"));
        info.setEb(p.requireInt("E_b"));
        info.setEr(p.requireInt("E_r"));
        info.setTimer(p.requireInt("timer"));
        info.setMer(p.requireInt("M_er"));
        info.setVmer(p.requireInt("V_mer"));
        info.setMvmer(p.requireInt("M_vmer"));

        if (info.getTimer() <= 0)
            throw exceptions::ValidationException("timer deve ser positivo");

        infoRepo_.push(info);
    }

    // ---- Back -> Hardware (GET /config?id=) ----
    // Retorna o shape cru esperado pelo firmware (sem envelope).
    Json::Value buildConfig(int id) {
        auto cfg = infoRepo_.getConfig(id);
        Json::Value out;
        out["id"] = id;
        if (cfg.has_value()) {
            out["E_b"] = cfg->getEb();
            out["timer"] = cfg->getTimer();
        } else {
            // Sem config ainda: defaults seguros equivalentes ao firmware.
            out["E_b"] = 0;
            out["timer"] = 10000;
        }
        return out;
    }

    // ---- Site: ver informacoes detalhadas ----
    Json::Value currentInfo(int id) {
        auto info = infoRepo_.findById(id);
        if (!info.has_value())
            throw exceptions::NotFoundException("Ainda nao ha leituras deste hardware");
        return info->toJson();
    }

    // ---- Site: manipular hardware (gravar config) ----
    void setConfig(int id, int E_b, int timer) {
        if (timer <= 0)
            throw exceptions::ValidationException("timer deve ser positivo");
        if (E_b < 0)
            throw exceptions::ValidationException("E_b nao pode ser negativo");
        infoRepo_.setConfig(id, E_b, timer);
    }

private:
    repository::InfoRepository infoRepo_;
};

} // namespace app::service
