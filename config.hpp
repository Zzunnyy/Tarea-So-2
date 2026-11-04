#pragma once
#include <fstream>
#include <utility>
#include <iostream>
#include <string>
#include <sstream>
#include <set>
#include <vector>
#include <cstdlib>
#include <cerrno>
#include "tablero.hpp"
using namespace std;

const int NUM_PERROS = 12;
// valores de la configuracion que ya usabamos
struct Config {
    vector<pair<int,int>> abandonos;
    int min_ms=50;
    int max_ms=300;
    unsigned int seed=42;
    int puma = NODO_T;
    vector<int> perros;
    Config(){
        for (int n = 10; n <=21; n++) perros.push_back(n);
    }
};

inline bool validar_config(const Config& c, string& error){
    if (c.min_ms < 0 || c.min_ms > c.max_ms){
        error = "min_ms >=0 y <= max_ms";
        return false;
    }
    if (c.puma < 0 || c.puma >= NUM_NODOS){
        error =" Casilla fuera de rnago";
        return false;
    }
    if ((int)c.perros.size() != NUM_PERROS){
        error = "se necesitan " + to_string(NUM_PERROS) + "casillas de perros";
        return false;
    }
    set <int> usadas;
    usadas.insert(c.puma);
    for (int n : c.perros){
        if (n<0 || n>= NUM_NODOS){
            error = "Casilla de perro fuera de rango" + to_string(n);
            return false;
        }
        if (!usadas.insert(n).second){
            error = "casilla repetida:" + to_string(n);
            return false;
        }
    }
    set <int> con_abandono;
    for (const auto& ab: c.abandonos){
        if (ab.first < 1 || ab.second < 0){
            error = "abandona: el jugador debe ser mayor a 1 y los ms mayor a 0";
            return false;
        }
        if (!con_abandono.insert(ab.first).second){
            error = "abandona: jugador repetido "+ to_string(ab.first);
            return false;
        }
    }
    return true;
}

inline bool cargar_config(const string& ruta, Config& c, string& error){
    ifstream f(ruta);
    if(!f){
        error = "No se pudo abrir" + ruta + "'"; //eso es para ver la ruta en caso de error
        return false;
    }
    string linea;
    int nro= 0;
    while(getline(f,linea)){
        nro++;
        size_t h= linea.find ('#');
        if (h!= string::npos) linea.erase(h);
        istringstream ss(linea);
        string clave;
        if (!(ss >> clave)) continue;

        string donde = "linea "+ to_string(nro) + ":";
        if (clave == "min_ms" || clave == "max_ms" || clave == "puma"){
            int v;
            string resto;
            if (!(ss >> v) || (ss >> resto)){
                error= donde + " "+ clave + "' necesita un solo numero entero";
                return false;
            }
            if(clave == "min_ms") c.min_ms = v;
            else if (clave== "max_ms")c.max_ms=v;
            else c.puma = v;
        }else if (clave == "seed"){
            long long v; 
            string resto;
            if (!(ss>>v)|| v < 0 || (ss>> resto)){
                error= donde + "'seed' necesita un numero >= 0";
                return false;
            }
            c.seed=(unsigned int)v;
        }else if (clave == "perros"){
            c.perros.clear();
            int v; 
            while (ss>> v)c.perros.push_back(v);
            if (!ss.eof()){
                error = donde +"'Perros' solo toma numeros enteros";
                return false;
            }
        }else if(clave == "abandona"){
            int jug, ms;
            
            string resto;
            if (!(ss >> jug >>ms) || (ss>>resto)){
                error = donde + "Abandona necesita dos enteros: jugador y ms";
                return false;
            }
            c.abandonos.push_back({jug,ms});
        }else{
            error = donde + "clave desconocida"+ clave + "'";
            return false;
        }
    }
    return validar_config(c,error);
}
inline bool leer_entero_positivo(const char* texto, int& salida){
    char* fin = nullptr;
    errno=0;
    long v = strtol(texto, &fin, 10);
    if (errno !=0 || fin==texto || *fin != '\0' || v <= 0 || v > 1000000) return false;
    salida = (int)v;
    return true;
}
inline int ms_abandono(const Config& c, int jugador){
    for (const auto& ab : c.abandonos)
        if (ab.first == jugador) return ab.second;
    return -1;
}
