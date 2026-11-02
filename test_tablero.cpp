#include <iostream>
#include "tablero.hpp"
using namespace std;

int main(){
    auto vecinos = construir_vecinos();
    auto saltos = construir_saltos();
    int suma = 0;
    for (int i =0; i< NUM_NODOS; i++){
        cout<< i << ": ";
        for (int v : vecinos[i]) cout << v << " ";
            cout << "\n";
            suma+= vecinos[i].size();
        }
    cout << "aristas: " << suma/2 << "\n";
    cout << "saltos: " << saltos.size() << "\n";
}
