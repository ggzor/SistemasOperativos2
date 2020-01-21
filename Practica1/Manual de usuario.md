# Manual de usuario

## Índice

0. [Notas](#0.-notas)
1. [Compilación](#1.-compilación)
2. [Generación de procesos](#2.-Generación-de-procesos)
3. [Ejecución del planificador](#3.-Ejecución-del-planificador)  
  a. [Tiempo virtual](#3.a-Tiempo-virtual)  
  b. [Invocación de planificadores](#3.b-Invocación-de-planificadores)
4. [Salida](#4.-Salida)

## 0. Notas

Este trabajo fue realizado por:
  - [Axel Suárez Polo](https://github.com/ggzor/)
  - [Sergio Cortez Chávez](https://github.com/SerCor)

> El repositorio de **GitHub** para este proyecto se puede acceder desde [aquí](https://github.com/ggzor/SistemasOperativos2/), a dónde se puede encontrar el código fuente más actualizado del proyecto. Se aceptan Pull Requests que definitivamente serán consideradas, no se prevé mover el repositorio pronto.

## 1. Compilación

> **Nota:** El programa está hecho para ser compilado y ejecutado en un entorno de ejecución **UNIX**. El programa no funciona en el sistema operativo Windows.

Para utilizar el programa se requiere compilar el código fuente utilizando el script de bash `compilarTodo`.

```bash
$ ./compilarTodo
```

Una vez que se ha realizado la compilación, los archivos ejecutables son colocados en la carpeta `bin`, pero es importante resaltar que estos programas se deben ejecutar desde la carpeta superior.

## 2. Generación de procesos

Para la ejecución del generador de procesos se debe llamar al programa `bin/generador`, como se índica a continuación. Nótese que el programa genera los procesos y los coloca en la salida estándar por lo que es necesario redirigir la salida a un archivo para tener los procesos en un archivo.

Una característica del generador de procesos aleatorios es que permite seleccionar la semilla con la que se generan los procesos, lo que permite generaciones de procesos deterministas.

```bash
# Mostrar la ayuda del comando
$ bin/generador
Uso: ./generador <cantidad> [semilla=0]
  Genera <cantidad> procesos utilizando la [semilla] 
  dada (0 por defecto).
  Los resultados se ponen en la salida estándar.

# Un ejemplo de salida (que de seguro coincide con la tuya ;)
$ bin/generador 3
1 20 2
2 42 1
3 34 1

# Generar 100 procesos con la semilla de números
# pseudoaleatorios 0 (es el default) y escribirlo
# en el archivo "test100"
$ bin/generador 100 > test100

# Generar 200 procesos con la semilla de números
# pseudoaleatorios 31 y escribirlo en "test200"
$ bin/generador 200 31 > test200
```

> **Nota:** Se recomienda usar nombres de archivo sin extensión para los archivos generados, ya que el nombre de archivo forma parte del nombre de la salida al ejecutar un planificador.

## 3. Ejecución del planificador

Para ejecutar alguno de los planificadores que se incluyen, se debe llamar al programa `bin/main`. A continuación se detalla su funcionamiento.

```bash
# Mostrar información general del programa
$ bin/main
ERROR: No se proporcionaron los argumentos necesarios.
Uso: bin/main [-s semilla=0] [-t] <planificador> <lista-procesos>
  Planificadores disponibles:
    fifo: Planificador FIFO
    sjf:  Trabajo más corto primero
    sjfp: Proceso con mayor prioridad primero
    rr:   Round-robin con quantum estatico 
    rre:  Round-robin por epocas con quantum y vidas dependiente de prioridad
    srtf: El trabajo con el menor tiempo restante primero
    lpp:  Lotería por prioridad

  Opciones:
    -s    La semilla es para la generación de números aleatorios (0 por defecto)
    -t    Realizar la simulación utilizando tiempo virtual, es decir
          usando sólo contadores y no "sleep"
```

Tal y como se detalla en la salida, se han implementando múltiples planificadores que se detallan en el archivo **"Descripción del proyecto"**.

### **3.a Tiempo virtual**

Hay una carácterística bastante importante, que es la de **tiempo virtual** (**vtime**). Esta misma se detalla en la **"Descripción del proyecto"**. Básicamente, lo que permite es realizar simulaciones precisas de los tiempos de ejecución de los planificadores en un tiempo bastante corto. Esta puede ser activada pasando la opción `-t` en la línea de comandos.

### **3.b Invocación de planificadores**

A continuación se muestran ejemplos de la ejecución de los planificadores.

```bash
# Ejecutar el planificador rre (Round-Robin con épocas)
# con el archivo de procesos test5
# utilizando tiempo real
$ bin/main rre test5
...

# Ejecutar el planificador fifo con el archivo de procesos test5
# utilizando tiempo real
$ bin/main fifo test5
...

# Ejecutar el planificador srtf (tiempo restante más corto)
# con el archivo de procesos test200
# utilizando tiempo virtual y semilla 30
$ bin/main -s 30 -t srtf test200
...
```

Durante la ejecución del programa, se encuentran básicamente dos tipos de notificaciones en la salida estándar:

- **Notificación de envío de ráfagas de procesos:** Registra cuántos y cuáles procesos se enviaron en la ráfaga.
  ```
  Numero de procesos enviados en la rafaga: 2
  38 71 3
  39 135 5
  ```

- **Notificación de uso del despachador:** Registra que proceso utilizó el despachador y por cuanto tiempo.

  ```text
  Ejecutando 10 por 5s
  ```

## 4. Salida

El archivo generado por el programa tiene su nombre en el siguiente formato:

  ```text
  salida-(planificador)-(entrada)-(tipo-tiempo).txt
  ```

Donde: 

- **(planificador)** es el planificador elegido.
- **(entrada)** es el nombre del archivo de entrada.
- **(tipo-tiempo)** es `vtime` si se ejecutó con tiempo virtual o `real` si se ejecutó con tiempo real.

El contenido de la salida tiene una forma similar a la siguiente:

```text
Cantidad procesos: 25
Tiempo total: 2092
Espera total: 15914
Espera promedio: 636.56

Estadísticas por prioridad:
Prioridad   Procesos   Espera-Promedio   Espera-Total
    1           5          445.00             2225
    2           6          678.83             4073
    3           6          945.17             5671
    4           4          369.25             1477
    5           4          617.00             2468

PID Tiempo Prioridad Espera Total
  7     11     5          4    15
 11      8     4         15    23
  1     20     2         19    39

```

La mayoría de las estadísticas son auto-explanatorias, solo cabe resaltar que los procesos se van agregando a la lista en el orden en el que terminan y que todos las medidas de tiempo están en segundos.
