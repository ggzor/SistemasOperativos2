# Descripción de Práctica 1

<!-- #TODO: Agregar algun componente faltante.(TIEMPO VIRTUAL) -->

## Indice

0. [Notas](#0.-Notas)
1. [Lector de procesos](#1.-Lector-de-procesos)
2. [Planificadores](#2.-Planificadores)  
    a. [Planificador de corto plazo](#2.a-Planificador-de-corto-plazo)  
    b. [Planificador de largo plazo](#2.b-Planificador-de-largo-plazo)
3. [Despachador](#3.-Despachador)

## 0. Notas

Este trabajo fue realizado por:

  - [Axel Suárez Polo](https://github.com/ggzor/)
  - [Sergio Cortez Chávez](https://github.com/SerCor)

> El repositorio de **GitHub** para este proyecto se puede acceder desde [aquí](https://github.com/ggzor/SistemasOperativos2/), a dónde se puede encontrar el código fuente más actualizado del proyecto.

## 1. Lector de procesos

**Actividades:**

+ Encargado de recolectar los procesos del archivo fuente y enviarlos en rafagas de 1-15 procesos cada 1-5 segundos.
+ Mantiene una tubería con el planificador de largo plazo, a través de ella envía las rafagas de procesos y notifica el final del envío por medio del cierre de dicha tubería.

**Archivos relacionados:**

+ `lectorProcesos.c.` Fichero fuente donde se define la recolección y tranmisión de los procesos desde el archivo origen hasta el planificador de largo plazo.

## 2. Planificadores

Parte central del programa. Es el conjunto de archivos fuentes que permiten definir las políticas del algoritmo de planificación. Todas las implementaciones de los algoritmos parten de un par de funciones definidas en la cabecera:
`planificador.h`.

    void recibir(Proceso *proceso);
    void operar(Nodo *lista);

### **2.a Planificador de largo plazo**

El planificador a largo plazo se centra principalmente en la implementación de la función recibir. Concretamente la función `recibir(Proceso *proceso)` abstrae la parte del planificador de largo plazo encargada de recibir un proceso desde el lector de procesos y que finalmente termina en el llamado a dicha función con el proposito de que en está se tenga toda lógica relacionada a como va ser la transmisión de los datos hacia el planificador de largo plazo y las políticas a implementar en la inserción del nuevo proceso en el área compartida entre ambos planificadores.

**Actividades:**

+ Mantiene comunicación con el lector de procesos para recibir y manipular las rafagas de procesos.
+ Hace uso de una región compartida y de un conjunto de semaforos para la sincronización y organización con el planificador de corto plazo. Esta implementación puede diferir entre la implementación de algunos algoritmos.
+ Es el encargado de suministrar procesos al planificador de corto plazo. Aplica algunas políticas del algoritmo de planificación para insertar en la región compartida.

### **2.b Planificador de corto plazo**

El planificador a corto plazo se centra principalmente en la implementación de la función operar. En particular la función
`void operar(Nodo *lista);` abstrae la parte del planificador del largo plazo encargada de la selección de un proceso enviado desde el planificador de largo plazo mediante las políticas del algoritmo planificador. En dicho método se lleva el control de las actualizaciones de los tiempos faltantes de cada proceso en el área compartida, así como la determinación del valor de quantum y otros detalles relativos al algoritmo.

**Actividades:**

+ Mantiene comunicación con el planificador de largo plazo y se encarga de seleccionar el siguiente proceso a ejecutar.
+ Tiene comunicación con el despachador, el cual es ejecutado una vez que se ha seleccionado el siguiente proceso
+ Lleva el control de las estadisticas de los procesos y su recolección.

**Archivos relacionados:**

+ `pipes.h.` Proporciona utilidades para la manipulación de la tubería con el lector de procesos.
+ `productorConsumidor.h.` Define la interfaz de métodos,macros y variables elementales que gran parte de los algoritmos de planificación implementan para tener una comunicación tanto con el lector de procesos como con planificador de largo plazo, así como asegurar el correcto manejo de las variables compartidas dando paso a macros como:

    1. `define accederMemoriaCompartida(code) {adquirirMutexMemoria(); code; liberarMutexMemoria();}`. Asegura la obtención y liberación del semáforo mutex dentro de la región crŕitica  de la memoria compartida
    2. `#define producir(code) {iniciarProduccion(); code; terminarProduccion();}`. Abstrae la parte de la inicialización de la producción en el planificador a largo plazo, asó como su terminación. Ambas partes manejan aspectos como la apertura de la tubería con el lector de procesos y la terminación de la producción cuando todos los procesos han sido enviados.
    3. `#define consumir(code) {terminado = iniciarConsumo(); code; terminarConsumo();}`. Abstrae la parte de iniciar y terminar consumo por parte del planificador a largo plazo.
    4. Etc.

+ `#TODO:fifo.c.` Implementación de los métodos operar y recibir con las políticas del algoritmo de planificación Fifo.
+ `sjf.c.` Implementación de los métodos operar y recibir con las políticas del algoritmo de planificación Shortest Job First. Consiste en que los procesos van a ser ordenados conforme a su tiempo de ejecución de manera que los procesos mas cortos van a ser ejecutados sin considerar un quantum, es decir, como un algoritmo no apropiativo.
+ `sjfp.c.` Implementación de los métodos operar y recibir con las políticas del algoritmo de planificación Shortest Job irst con la diferencia de que los procesos son ordenados conforme a su prioridad, de forma contraria a como se hacia tradicionalmente conforme al tiempo de ejecución.
+ `TODO:srtf.c.` Implementación de los métodos operar y recibir con las políticas del algoritmo de planificación El trabajo con.l menor tiempo restante primero.
+ `rr.c.` Implementación de los métodos operar y recibir con las políticas del algoritmo de planificación Round Robin. El planificador asigna equitativamente un quantum de tiempo a cada proceso en el orden de su llegada, esto se cicla hasta que progresivamente los procesos hayan sido completados.
+ `rre.c.` Implementación de los métodos operar y recibir con las políticas del algoritmo de planificación Round Robin con un tiempo de quantum variable dependiente de su prioridad, ademas incluye el concepto de epocas durante la cual los procesos tienen un número de vidas proporcional a su prioridad asignado al principio de cada epoca. La epoca se reiniciara al no quedar mas procesos con vidas consiguiendo que cada proceso reinicie sus vidas.
+ `#TODO:lpp.c` Implementación de los métodos operar y recibir con las políticas del algoritmo de planificación por lotería.

## 3. Despachador

Encargado de simular el área del Sistema Operativo que tiene como tarea preparar el cambio de contexto entre los registros de los procesos.

**Actividades:**

+ Simular mediante tiempo real o virtual, el tiempo de ejecución de un proceso en CPU.

**Archivos relacionados:**

+ `despachador.c` Implementación del despachador.

De un modo más técnico, el tiempo virtual proporciona un servicio de colocación de alarmas (`vsleep`) y control manual de tiempo multiprocesos sincronizado (`avanzarTiempo`), con la garantía de que para todas las alarmas `a1` y `a2`, si `a1 < a2` entonces `T(a1) <= T(a2)`, donde `T(x)` denota el tiempo verdadero en el que se ejecuta la alarma con tiempo esperado `x`.
