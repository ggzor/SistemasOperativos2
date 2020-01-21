# Descripción de pŕactica 1


## Índice

0. [Notas](#0.-notas)
1. [Lector de procesos](#1.-Lector-de-procesos)
2. [Planificadores](#2.-Planificadores)  
  a. [Planificador de largo plazo](#2.a-Planificador-de-largo-plazo)  
  b. [Planificador de corto plazo](#2.b-Planificador-de-corto-plazo)
3. [Despachador](#3.-Despachador)

## 0. Notas

Este trabajo fue realizado por:
  - [Axel Suárez Polo](https://github.com/ggzor/)
  - [Sergio Cortez Chávez](https://github.com/SerCor)

> El repositorio de **GitHub** para este proyecto se puede acceder desde [aquí](https://github.com/ggzor/SistemasOperativos2/), a dónde se puede encontrar el código fuente más actualizado del proyecto. Se aceptan Pull Requests que definitivamente serán consideradas, no se prevé mover el repositorio pronto.

## 1. Lector de procesos

**Actividades:**
+ Encargado de recolectar los procesos del archivo fuente y enviarlos en rafagas de 1-15 procesos cada 1-5 segundos. 
+ Mantiene una tubería con el planificador de largo plazo, a través de ella envía las rafagas de procesos y notifica el final del envío por medio del cierre de dicha tubería. 

**Archivos relacionados:**
+ `lectorProcesos.c.` Fichero fuente donde se define la recolección y tranmisión de los procesos desde el archivo origen hasta el planificador de largo plazo.


## 2. Planificadores

Parte central del programa. Es el conjunto de archivos fuentes que permiten definir las políticas del algoritmo de planificación. Todas las implementaciones de los algoritmos parten de un par de funciones definidas en la cabecera `planificador.h`.

    void recibir(Proceso *proceso);
    void operar(Nodo *lista);

### **2.a Planificador de largo plazo**

El planificador a largo plazo se centra principalmente en la implementación de la función recibir. Concretamente la función `recibir(Proceso *proceso)` abstrae la parte del planificador de largo plazo encargada de recibir un proceso desde el lector de procesos y que finalmente termina en el llamado a dicha función con el proposito de que en está se tenga toda lógica relacionada a como va ser la transmisión de los datos hacia el planificador de largo plazo y las políticas a implementar en la inserción del nuevo proceso en el área compartida entre ambos planificadores.

**Actividades:**

+ Mantiene comunicación con el lector de procesos para recibir y manipular las rafagas de procesos.
+ Hace uso de una región compartida y de un conjunto de semaforos para la sincronización y organización con el planificador de corto plazo. Esta implementación puede diferir entre la implementación de algunos algoritmos 
+ Es el encargado de suministrar procesos al planificador de corto plazo. Aplica algunas políticas del algoritmo de planificación para insertar en la región compartida.

### **2.b Planificador de corto plazo**

El planificador a corto plazo se centra principalmente en la implementación de la función operar. En particular la función
`void operar(Nodo *lista);` abstrae la parte del planificador del largo plazo encargada de la selección de un proceso enviado desde el planificador de largo plazo mediante las políticas del algoritmo planificador. En dicho método se lleva el control de las actualizaciones de los tiempos faltantes de cada proceso en el área compartida, así como la determinación del valor de quantum y otros detalles relativos al algoritmo.

**Actividades:**

+ Mantiene comunicación con el planificador de largo plazo y se encarga de seleccionar el siguiente proceso a ejecutar.
+ Tiene comunicación con el despachador, el cual es ejecutado una vez que se ha seleccionado el siguiente proceso.
+ Lleva el control de las estadisticas de los procesos.

**Archivos relacionados:**

+ `pipes.h.` Proporciona utilidades para la manipulación de la tubería con el lector de procesos
+ `productorConsumidor.h.` Define la interfaz de métodos,variables y macros, que gran parte de los algoritmos de planificación implementan para tener una comunicación segura y sincronizada entre los planificadores de corto y largo plazo. Algunos de los métodos y macros mas relevantes son:  

  - La macro `#define accederMemoriaCompartida(code) {adquirirMutexMemoria(); code; liberarMutexMemoria();}` proporciona una encapsulación del código correspondiente a la región crítica de la memoria compartida. Se encarga de el cierre y apertura del mutex.
  - La macro `#define consumir(code) {terminado = iniciarConsumo(); code; terminarConsumo();}` es la responsable de encapsular el código correspondiente al planificador de corto plazo. Proporciona la protección del código del planificador con su respectiva inicialización y fin del consumo de procesos.
  - La macro`#define producir(code) {iniciarProduccion(); code; terminarProduccion();}` es la encargada de prover al planificador de corto plazo de una inicialización y terminación de su producción.
  - La función `void iniciarProduccion();` y `void terminarProduccion();`son las encargadas de abstraer el inicio y terminación de la producción por parte del planificador a largo plazo. Dichas funciones encapsulan el inicio de la comunicación con el proceso lector, así como la comunicación del fin de la tranmisión hacia el consumidor(El planificador de corto plazo).
  - La función `int iniciarConsumo();` y `void terminarConsumo();` abstraen el inicio y fin del consumo por parte del planificador de corto plazo. Mediante estas funciones se logra la apertura del canal de comunicación entre el planificador a largo y corto plazo, así como la liberación de algunos recursos al final del consumo.

+ `fifo.c.` Implementación de los métodos operar y recibir con las políticas del algoritmo de planificación Fifo. Algoritmo no apropiativo mediante el cual los procesos son despachados en el mismo orden en el que llegaron.
+ `sjf.c.` Implementación de los métodos operar y recibir con las políticas del algoritmo de planificación Shortest Job First. Algoritmo no apropiativo mediante el cual los procesos son despachados conforme a su tiempo de ejecución, los procesos con menor tiempo de ejecución son despachados primero.
+ `sjfp.c.` Implementación de los métodos operar y recibir con las políticas del algoritmo de planificación Shortest Job irst con la diferencia de que los procesos son ordenados conforme a su prioridad sin tomar en cuenta su tiempo de ejecución. 
+ `srtf.c.` Implementación de los métodos operar y recibir con las políticas del algoritmo de planificación de el trabajo con el menor tiempo restante primero. 
+ `rr.c.` Implementación de los métodos operar y recibir con las políticas del algoritmo de planificación Round Robin. Algoritmo apropiativo mediante el cual los procesos son despachados conforme a su tiempo de llegada, asignandoles un quantum de tiempo estático por turno de cpu.
+ `rre.c.` Implementación de los métodos operar y recibir con las políticas del algoritmo de planificación Round Robin con un tiempo de quantum variable dependiente de su prioridad. Añade el concepto de epocas las cuales consisten en asignar un número finito de vidas a cada proceso, en donde el número de vidas debe ser proporcional a su prioridad(los procesos entrantes tienen 0 vidas). Una vez que no existan procesos en espera con número de vidas se realiza el reinicio de vidas.
+ `TODO:lpp.c` Implementación de los métodos operar y recibir con las políticas del algoritmo de planificación por lotería.


## 3. Despachador

Encargado de simular el área del Sistema Operativo que tiene como tarea preparar el cambio de contexto entre los registros de los procesos.

**Actividades:**
+ Simular mediante tiempo real o virtual, el tiempo de ejecución de un proceso en CPU.

**Archivos relacionados:**
+ `despachador.c` Implementación del despachador.