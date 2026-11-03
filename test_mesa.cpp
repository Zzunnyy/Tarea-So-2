#include <pthread.h>
#include <iostream>
#include "mesa.hpp"
using namespace std;

struct Intento{Mesa* m; int jugador; int destino; bool ok;};

void *hilo_intento(void *arg){
    Intento* intento = (Intento*) arg;
    intento->ok = intentar_mover(*intento->m, intento->jugador, intento->destino);
    return NULL;
}

int fallos = 0;

// imprime el resultado y cuenta la prueba como fallida si no coincide con lo esperado
void verificar(const string& nombre, int obtenido, int esperado){
    cout << nombre << obtenido << " (esperado " << esperado << ")\n";
    if (obtenido != esperado) fallos++;
}

int main(){
    //un solo hilo
    Mesa m;
    mesa_init(m);
    verificar("perro1 10->5 (libre, vecino): ", intentar_mover(m, 1, 5), 1);
    verificar("perro2 11->12 (ocupada):      ", intentar_mover(m, 2, 12), 0);
    verificar("perro2 11->3 (no vecino):     ", intentar_mover(m, 2, 3), 0);
    verificar("puma 25->26:                  ", intentar_mover(m, PUMA, 26), 1);
    mesa_imprimir(m);
    mesa_destruir(m);

    // Esto es una prueba de captura !!
    Mesa k;
    mesa_init(k);
    k.celda[10] = LIBRE; k.celda[26] = 1; k.pos[1] = 26;   // perro 1 en TI
    verificar("puma salta 25->26->0: ", intentar_saltar(k, 0), 1);
    verificar("capturados: ", k.capturados, 1);
    mesa_destruir(k);
        // esto es una prueba para puma acorralado
    Mesa t;
    mesa_init(t);
    for (int i = 0; i < NUM_NODOS; i++) t.celda[i] = LIBRE;
    for (int p = 1; p <= NUM_PERROS; p++) t.pos[p] = -1;
    t.celda[24] = PUMA;
    t.pos[PUMA] = 24;
    int sitios[4] = {23, 14, 22, 18};
    for (int i = 0; i < 4; i++) {
        t.celda[sitios[i]] = i + 1;
        t.pos[i + 1] = sitios[i];
    }
    verificar("ganador antes: ", t.ganador, 0);
    verificar("perro4 18->19: ", intentar_mover(t, 4, 19), 1);
    verificar("ganador despues: ", t.ganador, 2);
    mesa_destruir(t);
    // ejemplo pelear por casilla
    int malas = 0;
    for (int ronda = 0; ronda < 1000; ronda ++){
        Mesa r;
        mesa_init(r);
        for ( int i =0; i < NUM_NODOS; i++) r.celda[i] = LIBRE;
        for (int p = 1; p <= NUM_PERROS; p++) r.pos[p] = -1;
        r.celda[NODO_T] = PUMA; r.pos[PUMA] = NODO_T;
        int vecinosde12[4]= {7,11,13,17};
        for (int i=0; i<4; i++){ r.celda[vecinosde12[i]]= i +1; r.pos[i+1] = vecinosde12[i];}
        pthread_t hilos[4];
        Intento intentos[4];
        for (int i=0; i<4; i++){
            intentos[i]= {&r, i +1, 12, false};
            pthread_create(&hilos[i],  nullptr, hilo_intento, &intentos[i]);
        }
        int exitos = 0;
        for (int i = 0; i<4; i++){ pthread_join(hilos[i],nullptr); exitos += intentos[i].ok; }
        if (exitos != 1) malas++;
        mesa_destruir(r);
    }
    verificar("rondas con != ganador: ", malas, 0);

    if (fallos > 0){
        cout << fallos << " prueba(s) fallaron\n";
        return 1;
    }
    return 0;
}
