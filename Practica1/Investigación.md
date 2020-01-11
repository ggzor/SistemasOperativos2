# Investigación

## ¿Cómo se planean los procesos realmente?

- En Linux, se planean los procesos con una técnica de **compartición de tiempo**.
- Es importante señalar que la unidad mínima planeable de Linux es el **hilo** y no el proceso. Aunque es sobre-entendido que los procesos tienen generalmente un sólo hilo.
- Como se ha dicho, el hilo es la unidad mínima, pero no es la única. Linux puede planear grupos de hilos. A estas entidades se les llama **entidades planeables**.
- Los procesos son separados en dos clases: **Interactivos** y **No-Interactivos**. Aunque las clases no son mutuamente excluyentes.
- Los procesos interactivos son aquellos que utilizan continuamente operaciones de E/S.
- Para maximizar el uso de CPU y garantizar tiempos de respuesta rápidos, Linux tiende a asignar a:
  - **Procesos no interactivos:** largas rebanadas de tiempo consecutivas pero poco frecuentes.
  - **Procesos interactivos:** cortas rebanadas de tiempo, pero muy frecuentes.
- Todos los procesos son planeados de acuerdo a una de las clases: **Batch, Idle, FIFO, RoundRobin o Convencional**. Ver página 17 de [2].
- Linux implementa una planeación basada en prioridades. Un proceso con mayor prioridad se ejecuta antes que uno con menor prioridad. 
- Cada proceso convencional tiene su propia prioridad estática, que es un número en el rango del 100 al 139. Esta prioridad determina el tiempo en ms que se le asignará a 

Fuentes:

1. [Linux Kernel Process Scheduling (Medium)](https://medium.com/hungys-blog/linux-kernel-process-scheduling-8ce05939fabd)
2. [A complete guide to Linux process Scheduling de Nikita Ishkov](https://trepo.tuni.fi/bitstream/handle/10024/96864/GRADU-1428493916.pdf)

## ¿Qué tan caro es redimensionar la memoria compartida?

## ¿Qué es más caro? 2 Pipes o 1 socket local
