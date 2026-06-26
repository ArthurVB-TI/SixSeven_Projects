#include "Dados.cpp"
#include "Hooks.cpp"
#include "hardware.cpp"

// Constantes
const int PIN_led = 0;
const int PIN_interruptor = 0;


int isLigado(){
    
}

int init_conection(){
    int retorno = 0;
    
    return retorno;
}

void await(int time){

}


void setup(){
    
}

void loop(){
    if(isLigado()){
        if(init_conection()){
            Dados* d = new Dados();
            await(d->get_timer());
        }
    }
}