# Tarea 2: Komikan Concurrente

Integrantes: Maximiliano Santibañez, Benjamín Zuñiga.

El programa simula un torneo de Komikan con un puma y 12 perros por mesa. Cada pieza tiene su propio hilo y se mueve sin turnos, con pausas aleatorias entre los intentos.

## Compilación y ejecución
Desde la carpeta del repositorio se compila con:

```bash
g++ -Wall -Wextra -std=c++17 -pthread torneo.cpp -o torneo
```

El programa se ejecuta como `./torneo N M [config.txt]`, donde N es la cantidad de jugadores y M el max de mesas activas al mismo tiempo. Ambos deben ser enteros positivos. El archivo de configuración es opcional. Por ejemplo:

```bash
./torneo 8 2 config.txt
```

Esto procesa ocho jugadores con un máximo de dos mesas simultáneas. La consola muestra el estado de las partidas y un resumen al terminar. 

## Config

El archivo usa una línea por parámetro, con el nombre seguido de sus valores. Se pueden agregar comentarios con `#`. El archivo incluido contiene:

```text
min_ms 50
max_ms 300
seed 42
puma 25
perros 10 11 12 13 14 15 16 17 18 19 20 21
```

min_ms y max_ms indican la espera mínima y máxima antes de intentar un movimiento, en milisegundos. Deben cumplir 0 <= min_ms <= max_ms. seed es una semilla aleatoria no negativa. puma indica su casilla inicial y perros debe indicar 12 casillas distintas, sin coincidir con el puma. Si no se entrega un archivo, se usan los valores del ejemplo.

Las casillas de la cuadrícula van de 0 a 24, ordenadas por filas desde arriba a la izquierda. Las posiciones del trono son 25 (T), 26 (TI) y 27 (TD).

También se puede agregar abandona jugador ms para simular que un jugador abandona después de ese tiempo desde el inicio de su mesa. El identificador debe ser positivo, el tiempo no negativo y no se puede repetir un jugador. Para probarlo está el archivo config_abandono.txt:

```bash
./torneo 8 2 config_abandono.txt
```

## Sincronización

Usamos un mutex por mesa para proteger el tablero. Cada movimiento se revisa y se aplica con ese mutex tomado, incluyendo las capturas y la comprobación de victoria. Así, si dos fichas quieren la misma casilla, solo una puede ocuparla. Elegimos este diseño porque es sencillo de manejar, aunque obliga a aplicar los movimientos de una mesa de uno en uno. Las distintas mesas sí pueden avanzar al mismo tiempo.

El servidor tiene otro mutex para la fila de jugadores y el contador de mesas. Las variables de condición permiten esperar por una pareja o una mesa libre sin mantener ocupado ese mutex. Además, un mutex separado evita que se mezclen los mensajes de consola. Al cerrar, se espera a que terminen los hilos antes de destruir los recursos de sincronización.

El análisis de los riesgos y las decisiones del diseño está en el informe pdf de la raiz del repo
