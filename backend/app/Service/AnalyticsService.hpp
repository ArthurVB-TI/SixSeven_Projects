#pragma once
// ============================================================
//  app/Service/AnalyticsService.hpp
//  Transforma a serie temporal (info_historico) em datasets
//  prontos para graficos e em metricas resumidas.
//
//  PADRAO STRATEGY: cada "grafico" e uma estrategia que sabe
//  extrair uma serie da lista de leituras. Adicionar um novo
//  grafico = adicionar uma estrategia, sem tocar no resto.
//  Isso resolve, de forma extensivel, o pedido de "varias
//  funcoes para mostrar os dados de forma interessante".
// ============================================================

#include <vector>
#include <string>
#include <memory>
#include <cmath>
#include <algorithm>
#include <json/json.h>
#include <drogon/orm/Row.h>
#include "../Repository/InfoRepository.hpp"

namespace app::service {

// Estrutura simples de leitura para as estrategias operarem.
struct Leitura {
    std::string t;   // timestamp
    int E_b, E_r, M_er, V_mer, M_vmer, timer;
};

// ---------------- Strategy base ----------------
class ChartStrategy {
public:
    virtual ~ChartStrategy() = default;
    virtual std::string key() const = 0;
    virtual std::string label() const = 0;
    // Constroi um dataset { labels:[...], series:[{name,data:[...]}] }.
    virtual Json::Value build(const std::vector<Leitura>& dados) const = 0;

protected:
    static Json::Value labelsOf(const std::vector<Leitura>& d) {
        Json::Value labels(Json::arrayValue);
        for (const auto& x : d) labels.append(x.t);
        return labels;
    }
};

// Grafico 1: Energia recebida x media (linha dupla).
class EnergiaVsMediaStrategy : public ChartStrategy {
public:
    std::string key() const override { return "energia_vs_media"; }
    std::string label() const override { return "Energia recebida x media"; }
    Json::Value build(const std::vector<Leitura>& d) const override {
        Json::Value serieEr(Json::arrayValue), serieMer(Json::arrayValue);
        for (const auto& x : d) { serieEr.append(x.E_r); serieMer.append(x.M_er); }
        Json::Value out;
        out["labels"] = labelsOf(d);
        Json::Value s(Json::arrayValue);
        s.append(named("E_r (recebida)", serieEr));
        s.append(named("M_er (media)", serieMer));
        out["series"] = s;
        return out;
    }
};

// Grafico 2: Variacao e media da variacao (estabilidade).
class VariacaoStrategy : public ChartStrategy {
public:
    std::string key() const override { return "variacao"; }
    std::string label() const override { return "Variacao (estabilidade)"; }
    Json::Value build(const std::vector<Leitura>& d) const override {
        Json::Value v(Json::arrayValue), mv(Json::arrayValue);
        for (const auto& x : d) { v.append(x.V_mer); mv.append(x.M_vmer); }
        Json::Value out;
        out["labels"] = labelsOf(d);
        Json::Value s(Json::arrayValue);
        s.append(named("V_mer", v));
        s.append(named("M_vmer", mv));
        out["series"] = s;
        out["threshold"] = 150; // limiar de estabilidade (firmware)
        return out;
    }
};

// Grafico 3: Percentual de energia (E_r / E_b) ao longo do tempo.
class PercentualStrategy : public ChartStrategy {
public:
    std::string key() const override { return "percentual"; }
    std::string label() const override { return "Percentual de energia"; }
    Json::Value build(const std::vector<Leitura>& d) const override {
        Json::Value pct(Json::arrayValue);
        for (const auto& x : d) {
            int p = (x.E_b > 0)
                ? static_cast<int>(std::lround((double)x.E_r / x.E_b * 100.0)) : 0;
            pct.append(p);
        }
        Json::Value out;
        out["labels"] = labelsOf(d);
        Json::Value s(Json::arrayValue);
        s.append(named("% energia", pct));
        out["series"] = s;
        return out;
    }
};

// ---------------- Servico que usa as estrategias ----------------
class AnalyticsService {
public:
    AnalyticsService() {
        strategies_.push_back(std::make_unique<EnergiaVsMediaStrategy>());
        strategies_.push_back(std::make_unique<VariacaoStrategy>());
        strategies_.push_back(std::make_unique<PercentualStrategy>());
    }

    // Retorna todos os graficos + um bloco de metricas resumidas.
    Json::Value dashboard(int idConection, int limite) {
        auto hist = infoRepo_.historico(idConection, limite);

        // O historico vem desc; para o grafico queremos ordem cronologica.
        std::vector<Leitura> dados = toLeituras(hist);
        std::reverse(dados.begin(), dados.end());

        Json::Value out;
        out["id_conection"] = idConection;
        out["pontos"] = static_cast<int>(dados.size());

        Json::Value charts(Json::arrayValue);
        for (const auto& s : strategies_) {
            Json::Value c;
            c["key"] = s->key();
            c["label"] = s->label();
            c["dataset"] = s->build(dados);
            charts.append(c);
        }
        out["charts"] = charts;
        out["resumo"] = resumo(dados);
        return out;
    }

    // Lista as estrategias disponiveis (para o front montar abas).
    Json::Value availableCharts() const {
        Json::Value arr(Json::arrayValue);
        for (const auto& s : strategies_) {
            Json::Value c;
            c["key"] = s->key();
            c["label"] = s->label();
            arr.append(c);
        }
        return arr;
    }

private:
    static Json::Value named(const std::string& name, const Json::Value& data) {
        Json::Value s;
        s["name"] = name;
        s["data"] = data;
        return s;
    }

    template <typename HistVec>
    static std::vector<Leitura> toLeituras(const HistVec& hist) {
        std::vector<Leitura> dados;
        dados.reserve(hist.size());
        for (const auto& h : hist) {
            Json::Value j = h.toJson();
            Leitura l;
            l.t = j["criado_em"].asString();
            l.E_b = j["E_b"].asInt();
            l.E_r = j["E_r"].asInt();
            l.M_er = j["M_er"].asInt();
            l.V_mer = j["V_mer"].asInt();
            l.M_vmer = j["M_vmer"].asInt();
            l.timer = j["timer"].asInt();
            dados.push_back(l);
        }
        return dados;
    }

    // Metricas resumidas: min/max/media de E_r, % estabilidade, ultimo estado.
    static Json::Value resumo(const std::vector<Leitura>& d) {
        Json::Value r;
        if (d.empty()) {
            r["vazio"] = true;
            return r;
        }
        int minEr = d[0].E_r, maxEr = d[0].E_r;
        long somaEr = 0;
        int estaveis = 0;
        for (const auto& x : d) {
            minEr = std::min(minEr, x.E_r);
            maxEr = std::max(maxEr, x.E_r);
            somaEr += x.E_r;
            if (std::abs(x.V_mer) <= 150) estaveis++;
        }
        r["vazio"] = false;
        r["e_r_min"] = minEr;
        r["e_r_max"] = maxEr;
        r["e_r_media"] = static_cast<int>(somaEr / (long)d.size());
        r["pct_estavel"] = static_cast<int>(std::lround((double)estaveis / d.size() * 100.0));
        r["ultimo_estavel"] = std::abs(d.back().V_mer) <= 150;
        r["ultimo_percentual"] = (d.back().E_b > 0)
            ? static_cast<int>(std::lround((double)d.back().E_r / d.back().E_b * 100.0)) : 0;
        return r;
    }

    repository::InfoRepository infoRepo_;
    std::vector<std::unique_ptr<ChartStrategy>> strategies_;
};

} // namespace app::service
