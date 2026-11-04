#pragma once
#include <pthread.h>
#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <atomic>
#include <csignal>
#include <sstream>
#include "mesa.hpp"
using namespace std;

const int TIMEOUT_MESA_S = 10; //esto evita partidas infinitaaas

inline pthread_mutex_t MTX_LOG = PTHREAD_MUTEX_INITIALIZER;
inline atomic<bool> g_cerrar{false};
inline void manejador_sigint(int){
    g_cerrar.store(true);
}
inline void log_texto(int id_mesa, const string& texto){
    pthread_mutex_lock(&MTX_LOG);
    istringstream in(texto);
    string linea;
    while (getline(in,linea)) cout << "[mesa" << id_mesa<< "]"<< linea << endl;
    cout.flush();
    pthread_mutex_unlock(&MTX_LOG);
}
struct ArgPieza{
    Mesa* m;
    int id; unsigned int seed;
    int min_ms, max_ms;
    int id_mesa;
};

inline void* hilo_pieza(void* p){
    ArgPieza* a = (ArgPieza*) p;
    Mesa& m= *a->m;
    mt19937 rng(a->seed + a->id);
    uniform_int_distribution<int> espera(a->min_ms, a->max_ms);

    while (!mesa_terminada(m)){
        this_thread::sleep_for(chrono::milliseconds(espera(rng))); 
        int pos = mesa_pos(m,a->id);
        if (pos == -1) break;
        if (a-> id == PUMA){
            vector<int> saltos;
            for(const Salto& s : SALTOS)
                if (s.desde == pos ) saltos.push_back(s.hasta);

            if (!saltos.empty() && rng() % 2 == 0){
                int destino = saltos[rng() % saltos.size()];
                if (intentar_saltar(m,destino))
                log_texto(a->id_mesa, "E: el puma captura y cae en la casilla " + to_string(destino));
            }else {
                const vector<int>& v = VECINOS [pos];
                intentar_mover(m,PUMA, v[rng() % v.size()]);
            }
        }else {
            const vector <int> & v = VECINOS[pos];
            intentar_mover(m,a->id, v[rng() % v.size()]);
        }
    }
    return nullptr;
}
struct DatosMesa{
    int jug_puma=0;
    int jug_perros =0;
    int abandona_puma_ms=-1;
    int abandona_perros_ms= -1;
    bool hubo_abandono= false;
};

inline int jugar_mesa(int id_mesa, const Config& cfg, DatosMesa& datos){
    Mesa m; 
    mesa_init(m,cfg);

    ArgPieza args [NUM_PERROS +1];
    pthread_t hilos[NUM_PERROS +1];
    unsigned int base = cfg.seed + (unsigned int) id_mesa *100u;
    log_texto(id_mesa, "mesa iniciada");
    int creados = 0;
    for (int i = 0; i <= NUM_PERROS; i++){
        args[i] = {&m, i, base, cfg.min_ms, cfg.max_ms, id_mesa};
        if (pthread_create(&hilos[i], nullptr, hilo_pieza, &args[i]) != 0){
            log_texto(id_mesa, "ERROR: no se pudo crear el hilo de la pieza " + to_string(i));
            mesa_terminar(m, 0);
            break;                                  // solo se hace join de los que existen
        }
        creados++;
    }   
    auto inicio = chrono::steady_clock::now();
    while (!mesa_terminada(m)){
        this_thread::sleep_for(chrono::milliseconds(500));
        log_texto(id_mesa, mesa_a_texto(m));
        long ms = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - inicio).count();
        int abandona = 0;
        if (datos.abandona_puma_ms >= 0 && ms >= datos.abandona_puma_ms) abandona = 1;
        if (datos.abandona_perros_ms >= 0 && ms >= datos.abandona_perros_ms && (abandona == 0 || datos.abandona_perros_ms < datos.abandona_puma_ms)) abandona = 2;
        if (g_cerrar){
            if (mesa_terminar(m, 0))
                log_texto(id_mesa, "EVENTO: el servidor fue interrumpido (SIGINT), se cierra la mesa");
        } else if (abandona == 1){
            if (mesa_terminar(m, 2)){
                datos.hubo_abandono = true;
                log_texto(id_mesa, "EVENTO: el jugador " + to_string(datos.jug_puma) +
                            " (puma) abandona, ganan los perros");
            }
        } else if (abandona == 2){
            if (mesa_terminar(m, 1)){
                datos.hubo_abandono = true;
                log_texto(id_mesa, "EVENTO: el jugador " + to_string(datos.jug_perros) +
                            " (perros) abandona, gana el puma");
            }
        } else if (ms > TIMEOUT_MESA_S * 1000L){
            if (mesa_terminar(m, 0))
                log_texto(id_mesa, "EVENTO: tiempo agotado, se cierra la mesa");
        }
    
    }
    for (int i = 0; i < creados; i++) pthread_join(hilos[i], nullptr);

    int ganador = m.ganador;        // seguro sin lock: ya se hizo join de todos los hilos
    log_texto(id_mesa, mesa_a_texto(m));
    log_texto(id_mesa, string("FIN: ") + (ganador == 1 ? "gana el puma" :ganador == 2 ? "ganan los perros" : "sin ganador (cierre anticipado)") +(datos.hubo_abandono ? " por abandono" : ""));
    mesa_destruir(m);
    return ganador;
}
inline int jugar_mesa(int id_mesa, const Config& cfg){
    DatosMesa datos;
    return jugar_mesa(id_mesa, cfg, datos);
}

struct ArgJuez {
    int id_mesa;
    const Config* cfg;
    int ganador;
};
inline void* hilo_juez(void* p){
    ArgJuez* a = (ArgJuez*) p;
    a->ganador = jugar_mesa(a-> id_mesa, *a->cfg);
    return nullptr;
}
