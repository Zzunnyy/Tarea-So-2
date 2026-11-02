#pragma once
#include <vector>
#include <iostream>
using namespace std;

//conestantes
const int FILAS = 5;
const int COLS = 5;
const int NODO_T= 25;
const int NODO_TI=26;
const int NODO_TD=27;
const int NUM_NODOS = 28;


struct Salto{
    int desde;
    int sobre;
    int hasta;
};

inline int id_grilla( int fila, int col){ return fila *COLS + col; }
// esto permite ver en ambos sentidos
inline void conectar(vector<vector<int>>& vecinos, int a, int b){
    vecinos[a].push_back(b);
    vecinos[b].push_back(a);
}

inline vector<vector<int>> construir_vecinos(){
    vector<vector<int>> vecinos(NUM_NODOS);
// esto conecta cada nodo con el de la derecha y el de abajo
    for (int f = 0; f< FILAS; f++){
        for (int c= 0; c< COLS; c++){
            if (c+1<COLS) conectar(vecinos, id_grilla(f,c), id_grilla(f,c+1));
            if (f+1<FILAS) conectar(vecinos, id_grilla(f,c), id_grilla(f+1,c));
        }
    }
// Trono

conectar(vecinos, NODO_T, NODO_TI);
conectar(vecinos, NODO_T, NODO_TD);
conectar(vecinos,NODO_T, id_grilla(0,2));
conectar(vecinos,NODO_TI, NODO_TD);
conectar(vecinos,NODO_TI, id_grilla(0,0));
conectar(vecinos,NODO_TI, id_grilla(0,1));
conectar(vecinos,NODO_TD, id_grilla(0,3));
conectar(vecinos,NODO_TD, id_grilla(0,4));

return vecinos;
}
inline void agregar_salto(vector<Salto>& saltos, int a, int b, int c){
    saltos.push_back({a,b,c});
    saltos.push_back({c,b,a});
}


inline vector<Salto> construir_saltos(){
    vector<Salto> saltos;
// estos son horizontales
    for (int f=0; f<FILAS; f++){
        for (int c=0; c+2<COLS; c++){
            agregar_salto(saltos, id_grilla(f,c), id_grilla(f,c+1), id_grilla(f,c+2));
        }
    }

    // estos son verticales
    for (int c=0; c<COLS; c++){
        for (int f=0; f+2<FILAS; f++){
            agregar_salto(saltos, id_grilla(f,c), id_grilla(f+1,c), id_grilla(f+2,c));
        }
    }
    agregar_salto(saltos,NODO_T, id_grilla(0,2), id_grilla(1,2));
    agregar_salto(saltos,NODO_TI, NODO_TI, id_grilla(0,0));
    agregar_salto(saltos,NODO_TD, NODO_TD, id_grilla(0,4));
    agregar_salto(saltos,NODO_TI, id_grilla(0,1), id_grilla(1,1));
    agregar_salto(saltos,NODO_TD, id_grilla(0,3), id_grilla(1,3));
    return saltos;


}