#ifndef FILTRO_MEDIA_MOVEL_H
#define FILTRO_MEDIA_MOVEL_H

#include <Arduino.h>

/*
 * FiltroMediaMovel — suaviza leituras ruidosas de sensores analogicos.
 *
 * Requisito OBRIGATORIO do edital (Filtro de Media Movel nos sinais analogicos).
 * Sem ele, o ruido do potenciometro/sensor causa ativacoes acidentais.
 *
 * Formula:  y[n] = (1/N) * soma das ultimas N leituras
 *
 * Usa BUFFER CIRCULAR para eficiencia O(1): em vez de somar as N posicoes
 * a cada leitura (O(N)), mantemos uma soma corrente e so trocamos o valor
 * mais antigo pelo mais novo. Exatamente a "Abordagem Ph.D." da pag. 22 do slide:
 *   SomaAtual = SomaAnterior - ValorAntigo + ValorNovo
 */
template <int N>
class FiltroMediaMovel {
  private:
    float _buffer[N];
    int _indice = 0;
    float _soma = 0;
    int _qtd = 0;   // quantos valores ja entraram (para a media ficar correta no inicio)

  public:
    FiltroMediaMovel() {
      for (int i = 0; i < N; i++) _buffer[i] = 0;
    }

    // Adiciona uma leitura nova e retorna a media atualizada.
    float adicionar(float valor) {
      _soma -= _buffer[_indice];   // remove o valor mais antigo da soma
      _buffer[_indice] = valor;    // grava o novo no lugar dele
      _soma += valor;              // adiciona o novo a soma
      _indice = (_indice + 1) % N; // anda o ponteiro circular
      if (_qtd < N) _qtd++;
      return media();
    }

    float media() {
      if (_qtd == 0) return 0;
      return _soma / _qtd;
    }
};

#endif
