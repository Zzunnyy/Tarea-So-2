#pragma once
#include <pthread.h>
#include "tablero.hpp"
#include <iostream>
#include <vector>
using namespace std;

const int LIBRE= -1;
const int PUMA= 0;
const int NUM_PERROS = 12;
const int CAPTURAS_PARA_GANAR =6;
// esto esta para lectura despues de construirse
inline const vector<vector<int>> VECINOS = construir_vecinos();
inline const vector<Salto> SALTOS = construir_saltos();

struct Mesa{
    int celda[NUM_NODOS];  // esto es para ver quien ocupa cada casilla
    int pos[NUM_PERROS + 1]; // donde esta cada pieza 
    int capturados;
    bool terminado;
    int ganador;    //0= nadie, 1= puma, 2=perros
    pthread_mutex_t mtx; // esto es una proteccion para todo lo anterior
};

inline void mesa_init(Mesa& m){
    for (int i = 0; i < NUM_NODOS; i++){
        m.celda[i] = LIBRE;
    }
    m.pos[PUMA] = NODO_T;
    m.celda[NODO_T] = PUMA;
    for (int p=1; p <= NUM_PERROS; p++){
        int nodo = 9 + p;
        m.pos[p] = nodo;
        m.celda[nodo] = p;
    }
    m.capturados = 0;
    m.terminado = false;
    m.ganador = 0;
    pthread_mutex_init(&m.mtx, nullptr);
}

inline void mesa_destruir(Mesa& m){
    pthread_mutex_destroy(&m.mtx);
}
inline bool puma_acorralado(const Mesa& m){
    int origen = m.pos[PUMA];
    for (int v : VECINOS[origen])
        if (m.celda[v] == LIBRE) return false;  //Puede Caminar
    for (const Salto& s : SALTOS)
        if (s.desde == origen && m.celda[s.sobre] >= 1 && m.celda[s.hasta] == LIBRE)
            return false;   // puede capturar
    return true;
}

// Movimiento simple a una casilla vecina vacia
inline bool intentar_mover(Mesa& m, int jugador, int destino){
    bool ok = false;
    pthread_mutex_lock(&m.mtx);
    if (!m.terminado && jugador >= 0 && jugador <= NUM_PERROS && m.pos[jugador] != -1 && destino >= 0 && destino < NUM_NODOS){
        int origen = m.pos[jugador];
        bool es_vecino = false;
        for (int v : VECINOS[origen]){
            if (v == destino){
                es_vecino = true;
                break;
            }
        }
        if (es_vecino && m.celda[destino] == LIBRE){
            m.celda[origen] = LIBRE;
            m.celda[destino] = jugador;
            m.pos[jugador] = destino;
            ok = true;
            if (puma_acorralado(m)){m.terminado = true; m.ganador = 2;}
        }
    }
    pthread_mutex_unlock(&m.mtx); //un solo desbloqueo al final
    return ok;

}

inline bool intentar_saltar( Mesa& m, int destino){
    bool ok = false; 
    pthread_mutex_lock(&m.mtx);
    if (!m.terminado){
        int origen = m.pos[PUMA];
        for (const Salto& s : SALTOS){
            if (s.desde == origen && s.hasta == destino && m.celda[s.sobre] >= 1 && m.celda[destino]== LIBRE){
                int perro = m.celda[s.sobre];
                m.celda[origen] = LIBRE;
                m.pos[perro] = -1;
                m.celda[s.sobre] = LIBRE;
                m.celda[destino] = PUMA;
                m.pos[PUMA] = destino;
                m.capturados++;
                ok = true;
                if (m.capturados >= CAPTURAS_PARA_GANAR){
                    m.terminado = true;
                    m.ganador = 1;
                } else if (puma_acorralado(m)){
                    m.terminado = true;
                    m.ganador = 2;
                }
                break;
            }
        }
    }
    pthread_mutex_unlock(&m.mtx);
    return ok;
}
inline void mesa_imprimir(Mesa& m){
    pthread_mutex_lock(&m.mtx);
    auto c = [&] (int nodo ){
        int x = m.celda [nodo];
        return x == LIBRE ? '.' : (x == PUMA ? 'P': 'D');
    };
    cout << " T="<< c(NODO_T)<< endl;
    cout << " TI="<< c(NODO_TI)<< " TD="<< c(NODO_TD)<< endl;
    for (int f= 0; f < FILAS; f++){
        for (int col=0; col < COLS; col ++) cout << c(id_grilla(f,col)) << ' ';
        cout << endl;
    }
    cout << "Capturados: " << m.capturados << endl;
    pthread_mutex_unlock(&m.mtx);
}