# Descripción de pŕactica 1

<!-- #TODO: Agregar algun componente faltante.(TIEMPO VIRTUAL) -->
## Componentes Principales

1. Lector de Procesos
2. Planificadores
    2.1. Planificador de Corto Plazo
    2.2. Planificador de Largo Plazo
3. Despachador.


### 1. Lector de Procesos

Actividades:
+ Encargado de recolectar los procesos del archivo fuente y enviarlos en rafagas de 1-15 procesos cada 1-5 segundos. 
+ Mantiene una tubería con el planificador de largo plazo, a través de ella envía las rafagas de procesos y notifica el final del envío por medio del cierre de dicha tubería. 

Archivos relacionados:
+ `lectorProcesos.c.` Fichero fuente donde se define la recolección y tranmisión de los procesos desde el archivo origen
hasta el planificador de largo plazo.


### 2. Planificadores

Parte central del programa. Es el conjunto de archivos fuentes que permiten definir las políticas del algoritmo de planificación. Todas las implementaciones de los algoritmos parten de un par de funciones definidas en la cabecera:
`planificador.h`
    
    void recibir(Proceso *proceso);
    void operar(Nodo *lista);

#### 2.1. Planificador de Largo Plazo

El planificador a largo plazo se centra principalmente en la implementación de la función recibir. Concretamente la función `recibir(Proceso *proceso)` abstrae la parte del planificador de largo plazo encargada de recibir un proceso desde el lector de procesos y que finalmente termina en el llamado a dicha función con el proposito de que en está se tenga toda lógica relacionada a como va ser la transmisión de los datos hacia el planificador de largo plazo y las políticas a implementar en la inserción del nuevo proceso en el área compartida entre ambos planificadores.

Actividades:
+ Mantiene comunicación con el lector de procesos para recibir y manipular las rafagas de procesos.
+ Hace uso de una región compartida y de un conjunto de semaforos para la sincronización y organización con el planificador de corto plazo. Esta implementación puede diferir entre la implementación de algunos algoritmos 
+ Es el encargado de suministrar procesos al planificador de corto plazo. Aplica algunas políticas del algoritmo de planificación para insertar en la región compartida.

#### 2.2. Planificador de Corto Plazo

El planificador a corto plazo se centra principalmente en la implementación de la función operar. En particular la función
`void operar(Nodo *lista);` abstrae la parte del planificador del largo plazo encargada de la selección de un proceso enviado desde el planificador de largo plazo mediante las políticas del algoritmo planificador. En dicho método se lleva el control de las actualizaciones de los tiempos faltantes de cada proceso en el área compartida, así como la determinación del valor de quantum y otros detalles relativos al algoritmo.

Actividades:
+ Mantiene comunicación con el planificador de largo plazo y se encarga de seleccionar el siguiente proceso a ejecutar.
+ Tiene comunicación con el despachador, el cual es ejecutado una vez que se ha seleccionado el siguiente proceso
+ Lleva el control de las estadisticas de los procesos.

Archivos relacionados:
+ `pipes.h.` Proporciona utilidades para la manipulación de la tubería con el lector de procesos 
+ `productorConsumidor.h.` Define la interfaz de métodos elemental, que gran parte de los algoritmos de planificación implementan para tener una comunicación tanto con el lector de procesos como con planificador de largo plazo 
+ `fifo.c.` Implementación de los métodos operar y recibir con las políticas del algoritmo de planificación Fifo. 
+ `sjf.c.` Implementación de los métodos operar y recibir con las políticas del algoritmo de planificación Shortest Job First. 
+ `sjfp.c.` Implementación de los métodos operar y recibir con las políticas del algoritmo de planificación Shortest Job irst con la diferencia de que los procesos son ordenados conforme a su prioridad sin tomar en cuenta su tiempo. 
+ `srtf.c.` Implementación de los métodos operar y recibir con las políticas del algoritmo de planificación El trabajo con.l menor tiempo restante primero.
+ `rr.c.` Implementación de los métodos operar y recibir con las políticas del algoritmo de planificación Round Robin con un quantum estático.
+ `rre.c.` Implementación de los métodos operar y recibir con las políticas del algoritmo de planificación Round Robin con un tiempo de quantum variable dependiente de su prioridad. Incluye el concepto de epocas durante la cual los procesos tienen un número de vidas proporcional a su prioridad.
+ `lpp.c` Implementación de los métodos operar y recibir con las políticas del algoritmo de planificación por lotería.


### 3. Despachador

Encargado de simular el área del Sistema Operativo que tiene como tarea preparar el cambio de contexto entre los registros de los procesos.

Actividades:
+ Simular mediante tiempo real o virtual, el tiempo de ejecución de un proceso en CPU.

Archivos relacionados:
+ `despachador.c` Implementación del despachador.