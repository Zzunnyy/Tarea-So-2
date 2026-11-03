#include <pthread.h>
#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include "mesa.hpp"
using namespace std;

struct ArgJugador{
    Mesa* m;
    int id;
    unsigned int seed;
    int min_ms, max_ms;
};

void* hilo_pieza(void* p){
    ArgJugador* a = (ArgJugador*)p;
    Mesa& m = *(a->m);
    mt19937 rng(a->seed + a->id); // esto lo hice par aun generador por hilo, para que no se repitan los numeros aleatorios
    uniform_int_distribution<int> espera ( a->min_ms, a->max_ms);
    while (!mesa_terminada(m)){
    this_thread::sleep_for(chrono::milliseconds(espera(rng)));
    int pos = mesa_pos(m, a->id);
    if (pos == -1) break;
    if (a-> id == PUMA){
        vector <int> saltos; 
        for (const Salto& s : SALTOS){
            if (s.desde == pos) saltos.push_back(s.hasta);
        }
        if (!saltos.empty() && rng()%2 == 0){
            intentar_saltar(m, saltos[rng()%saltos.size()]);
        } else {
            const vector<int> & v = VECINOS[pos];
            intentar_mover(m, PUMA, v[rng()%v.size()]);
        }
    }else {
        const vector<int> & v = VECINOS[pos];
        intentar_mover(m, a->id, v[rng()%v.size()]);
        }
    }
    return nullptr;
}
int main(int argc, char** argv){
    Config cfg;
    if (argc > 1){
        string error;
        if (!cargar_config(argv[1], cfg, error)){
            cerr << "Error en config:" << error << endl;
            return 1;
        }
    }
    Mesa m;
    mesa_init(m, cfg);
    ArgJugador args[NUM_PERROS+1];
    pthread_t hilos[NUM_PERROS+1];
    for (int i = 0; i <= NUM_PERROS; i++){
        args[i] = {&m, i, cfg.seed, cfg.min_ms, cfg.max_ms};
        pthread_create(&hilos[i], nullptr, hilo_pieza, &args[i]);
        }
    auto inicio = chrono::steady_clock::now();
    while (!mesa_terminada(m)){
        this_thread::sleep_for(chrono::milliseconds(500));
        mesa_imprimir(m);
        cout << "----" << endl;
        if (chrono::steady_clock::now() - inicio > chrono::seconds(10)){
            mesa_terminar(m, 0);
        }
    }
    for (int i =0; i<=NUM_PERROS; i++){
        pthread_join(hilos[i], nullptr);
    }
    mesa_imprimir(m);
    cout<< "Ganador:" << m.ganador <<" (0= nadie, 1= puma, 2=perros)" << endl;
    mesa_destruir(m);
}