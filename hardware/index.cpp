#include "config.hpp"

// Projeto ligado
int isLigado(){
    int retorno = 0;

    return retorno;
}

// Conexão com bluetooth
int init_conection(){
    int retorno = 0;
    
    return retorno;
}

// Tempo de espera
void await(int time){

}

// Definições iniciais
void setup(){
    
}


// Corpo
void loop(){
    if(isLigado()){
        if(init_conection()){
            Dados* d = new Dados();
            await(d->get_timer());

            // Pegar dados

            // Aplicar dados
        }
    }
}