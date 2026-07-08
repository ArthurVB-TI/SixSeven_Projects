#pragma once
// ============================================================
//  app/Models/Info.hpp
//  Entidade 'info': ultima leitura do hardware. Os nomes dos
//  campos espelham o payload do firmware (E_b, E_r, timer,
//  M_er, V_mer, M_vmer). O toJson expoe tambem metricas
//  derivadas (percentual e estabilidade) para a UI.
// ============================================================

#include <json/json.h>
#include <drogon/orm/Row.h>
#include <cmath>

namespace app::models {

class Info {
public:
    // Limiar de estabilidade equivalente ao firmware (STABLE_THRESHOLD).
    static constexpr int STABLE_THRESHOLD = 150;

    int getId() const { return id_; }
    int getEb() const { return E_b_; }
    int getEr() const { return E_r_; }
    int getTimer() const { return timer_; }
    int getMer() const { return M_er_; }
    int getVmer() const { return V_mer_; }
    int getMvmer() const { return M_vmer_; }

    void setId(int v) { id_ = v; }
    void setEb(int v) { E_b_ = v; }
    void setEr(int v) { E_r_ = v; }
    void setTimer(int v) { timer_ = v; }
    void setMer(int v) { M_er_ = v; }
    void setVmer(int v) { V_mer_ = v; }
    void setMvmer(int v) { M_vmer_ = v; }

    bool isStable() const { return std::abs(V_mer_) <= STABLE_THRESHOLD; }

    int percentualEnergia() const {
        if (E_b_ <= 0) return 0;
        return static_cast<int>(std::lround((static_cast<double>(E_r_) / E_b_) * 100.0));
    }

    Json::Value toJson() const {
        Json::Value j;
        j["id"] = id_;
        j["E_b"] = E_b_;
        j["E_r"] = E_r_;
        j["timer"] = timer_;
        j["M_er"] = M_er_;
        j["V_mer"] = V_mer_;
        j["M_vmer"] = M_vmer_;
        // metricas derivadas para a UI:
        j["estavel"] = isStable();
        j["percentual_energia"] = percentualEnergia();
        return j;
    }

    static Info fromRow(const drogon::orm::Row& row) {
        Info i;
        i.id_ = row["id"].as<int>();
        i.E_b_ = row["E_b"].as<int>();
        i.E_r_ = row["E_r"].as<int>();
        i.timer_ = row["timer"].as<int>();
        i.M_er_ = row["M_er"].as<int>();
        i.V_mer_ = row["V_mer"].as<int>();
        i.M_vmer_ = row["M_vmer"].as<int>();
        return i;
    }

private:
    int id_{0};
    int E_b_{0};
    int E_r_{0};
    int timer_{0};
    int M_er_{0};
    int V_mer_{0};
    int M_vmer_{0};
};

} // namespace app::models
