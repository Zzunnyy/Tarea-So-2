#include "juego.hpp"
using namespace std;

int main(int argc, char** argv){
    Config cfg;
    int num_mesas = 1;
    if (argc > 1){
        string error;
        if (!cargar_config(argv[1], cfg, error)){
            cerr << "Error en config: " << error << endl;
            return 1;
        }
    }
    if (argc > 2 && !leer_entero_positivo(argv[2], num_mesas)){
        cerr << "num_mesas debe ser un entero positivo" << endl;
        return 1;
    }

    vector<ArgJuez> args(num_mesas);
    vector<pthread_t> jueces(num_mesas);
    int lanzados = 0;
    for (int i = 0; i < num_mesas; i++){
        args[i] = {i + 1, &cfg, 0};
        if (pthread_create(&jueces[i], nullptr, hilo_juez, &args[i]) != 0){
            cerr << "No se pudo crear el juez de la mesa " << i + 1 << endl;
            break;
        }
        lanzados++;
    }
    for (int i = 0; i < lanzados; i++) pthread_join(jueces[i], nullptr);

    for (int i = 0; i < lanzados; i++)
        cout << "mesa " << args[i].id_mesa << " -> ganador " << args[i].ganador << endl;
    return 0;
}