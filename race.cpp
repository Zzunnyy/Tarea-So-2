#include <pthread.h>
#include <stdio.h>
#include <iostream>
using namespace std;

int contador = 0;
pthread_mutex_t candado = PTHREAD_MUTEX_INITIALIZER;

void* trabajo(void*){
    for (int i = 0; i < 1000000; i++){
        pthread_mutex_lock(&candado);
        contador++;
        pthread_mutex_unlock(&candado);
    }
    return nullptr;
}
int main(){
    pthread_t h[4];
    for (int i = 0; i < 4; i++){
        pthread_create(&h[i], nullptr, trabajo, nullptr);
    }
    for (int i = 0; i < 4; i++){
        pthread_join(h[i], nullptr);
    }
    cout << contador << endl;
}