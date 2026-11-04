#include <queue>
#include <ctime>
#include "juego.hpp"
using namespace std;

struct Servidor{
    pthread_mutex_t mtx;
    pthread_cond_t hay_pareja; // esto avisa cuando llega un jugador nuevo
    pthread_cond_t hay_mesa;// avisa si hay cupo
    queue<int> espera;// jugadores que estan esperando 
    int mesas_activas;
    int limite;
    int max_simultaneas;
};

void servidor_init(Servidor& s, int limite){
    pthread_mutex_init(&s.mtx, nullptr);
    pthread_cond_init(&s.hay_pareja, nullptr);
    pthread_cond_init(&s.hay_mesa, nullptr);
    s.mesas_activas = 0;
    s.limite = limite;
    s.max_simultaneas = 0;
}
void servidor_destruir(Servidor& s){
    pthread_cond_destroy(&s.hay_mesa);
    pthread_cond_destroy(&s.hay_pareja);
    pthread_mutex_destroy(&s.mtx);
}
void log_servidor(const string& texto){ // mutex de log de las mesas
    pthread_mutex_lock(&MTX_LOG);
    cout << "[servidor] " << texto << "\n";
    cout.flush();
    pthread_mutex_unlock(&MTX_LOG);
}

// espera en la condicion como maximo 100 ms para poder revisar g_cerrar
// (se llama con el mutex tomado)
void esperar_cond(pthread_cond_t& cond, pthread_mutex_t& mtx){
    timespec limite;
    clock_gettime(CLOCK_REALTIME, &limite);
    limite.tv_nsec += 100L * 1000000L;
    if (limite.tv_nsec >= 1000000000L){
        limite.tv_sec++;
        limite.tv_nsec -= 1000000000L;
    }
    pthread_cond_timedwait(&cond, &mtx, &limite);
}

struct ArgLlegada{
    Servidor* srv;
    int id;
    unsigned int seed;
    int max_ms;
};

void* hilo_llegada(void* p){
    ArgLlegada* a = (ArgLlegada*) p;
    mt19937 rng(a->seed + a->id);
    uniform_int_distribution<int> demora(0, a->max_ms);

    this_thread::sleep_for(chrono::milliseconds(demora(rng)));
    if (g_cerrar) return nullptr; // el servidor ya no acepta jugadores

    log_servidor("llega el jugador " + to_string(a->id));
    Servidor& s = *a->srv;
    pthread_mutex_lock(&s.mtx);
    s.espera.push(a->id);
    pthread_cond_signal(&s.hay_pareja);
    pthread_mutex_unlock(&s.mtx);


    return nullptr;
}

struct ArgMesa{
    int id_mesa;
    const Config* cfg;
    Servidor* srv;
    int jug_puma;
    int jug_perros;
    int ganador;
    DatosMesa datos;
};

void* hilo_mesa(void* p){
    ArgMesa* a = (ArgMesa*) p;
    a->ganador = jugar_mesa(a->id_mesa, *a->cfg, a->datos);

    Servidor& s = *a->srv;
    pthread_mutex_lock(&s.mtx);
    s.mesas_activas--;
    pthread_cond_signal(&s.hay_mesa);
    pthread_mutex_unlock(&s.mtx);
    return nullptr;
}

int main(int argc, char** argv){
    if (argc < 3 || argc > 4){
        cerr << "Uso: ./torneo N M [config.txt]" << endl;
        return 1 ;
    }
    int N,M;
    if (!leer_entero_positivo(argv[1], N) || !leer_entero_positivo(argv[2], M)){
        cerr << "N y M deben ser enteros positivos" << endl;
        return 1;
    }
    Config cfg;
    if (argc == 4){
        string error;
        if (!cargar_config(argv[3], cfg, error)){
            cerr << "Error en la configuracion: " << error << endl;
            return 1;
        }
    }
    struct sigaction sa{};
    sa.sa_handler = manejador_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);

    Servidor srv;
    servidor_init(srv,M);
    vector<ArgLlegada> llegadas(N);
    vector<pthread_t> hilos_jugador(N);
    int jugadores=0;
    for (int i = 0; i<N; i++){
        llegadas[i] = {&srv, i +1, cfg.seed +1000u, 200};
        if (pthread_create(&hilos_jugador[i], nullptr, hilo_llegada, & llegadas[i])!=0){
            log_servidor("No se pudo crear al jugador "+ to_string (i+1));
            break;

        }
        jugadores++;
    }
    int num_mesas = jugadores / 2;
    vector<ArgMesa> mesas(num_mesas);
    vector<pthread_t> hilos_mesa(num_mesas);
    int lanzadas = 0;
    for (int k = 0; k < num_mesas && !g_cerrar; k++){
        pthread_mutex_lock(&srv.mtx);
        while (srv.espera.size() < 2 && !g_cerrar)
            esperar_cond(srv.hay_pareja, srv.mtx);
        if (g_cerrar){
            pthread_mutex_unlock(&srv.mtx);
            break;
        }
        int puma = srv.espera.front(); srv.espera.pop();
        int perros = srv.espera.front(); srv.espera.pop();

        while (srv.mesas_activas >= srv.limite && !g_cerrar)
            esperar_cond(srv.hay_mesa, srv.mtx);
        if (g_cerrar){
            pthread_mutex_unlock(&srv.mtx);
            log_servidor("cierre del servidor: se descarta la pareja " + to_string(puma) + " y " + to_string(perros));
            break;
        }
        srv.mesas_activas++;
        if (srv.mesas_activas > srv.max_simultaneas) srv.max_simultaneas = srv.mesas_activas;
        pthread_mutex_unlock(&srv.mtx);

        log_servidor("mesa " + to_string(k + 1) + ": jugador " + to_string(puma) +" (puma) vs jugador " + to_string(perros) + " (perros)");
        mesas[k] = {k + 1, &cfg, &srv, puma, perros, 0, {}};
        mesas[k].datos = {puma, perros, ms_abandono(cfg, puma), ms_abandono(cfg, perros), false};
        if (pthread_create(&hilos_mesa[k], nullptr, hilo_mesa, &mesas[k]) != 0){
            log_servidor("no se pudo crear la mesa " + to_string(k + 1));
            pthread_mutex_lock(&srv.mtx);
            srv.mesas_activas--;
            pthread_mutex_unlock(&srv.mtx);
            break;
        }
        lanzadas++;
    }
    if (g_cerrar)
        log_servidor("SIGINT: no se aceptan mas jugadores; se espera el cierre ordenado de las mesas activas");
    for (int i =0; i< jugadores; i++) pthread_join(hilos_jugador[i], nullptr);
    for (int k=0; k< lanzadas; k++) pthread_join(hilos_mesa[k], nullptr);
    int puma_gana = 0, perros_ganan = 0, sin_ganador = 0, abandonos = 0;
    for (int k = 0; k < lanzadas; k++){
        if (mesas[k].ganador == 1) puma_gana++;
        else if (mesas[k].ganador == 2) perros_ganan++;
        else sin_ganador++;
        if (mesas[k].datos.hubo_abandono)abandonos++;
    }
    while (!srv.espera.empty()){
        log_servidor("el jugador " + to_string(srv.espera.front()) + " se quedo sin pareja");
        srv.espera.pop();
    }
    cout << "resumen: " << jugadores << " jugadores, limite " << M << " mesas, "
        << lanzadas << " mesas jugadas, maximo simultaneo " << srv.max_simultaneas << endl
        << "         puma " << puma_gana << " / perros " << perros_ganan
<< " / no hay ganador " << sin_ganador << "\n         (por abandono: " << abandonos << ")" << endl;
    servidor_destruir(srv);
    return g_cerrar ? 130 : 0;
}
