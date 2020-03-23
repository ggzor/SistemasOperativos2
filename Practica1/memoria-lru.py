from sys import argv
import argparse
import collections

"""Clase que representa un manejador de memoria LRU"""
class LRU:
    def __init__(self, numero_paginas, numero_marcos):
        """numero_paginas: Máximo de paginas en la tabla de páginas
           numero_marcos:  Cantidad de marcos de la memoria física"""
        self.numero_paginas = numero_paginas
        self.numero_marcos = numero_marcos

        self.tabla_paginas = {}

        # Se utiliza un diccionario ordenado para simular LRU
        # ya que preserva el orden de inserción
        self.marcos_paginas = collections.OrderedDict()

        self.estadisticas = {
                'accesos': 0,
                'fallos_pagina': 0,
                'reemplazos': 0 }

    def pagina_lru(self):
        """Obtiene la entrada de la página menos recientemente utilizada"""
        return next(iter(self.marcos_paginas.values()))

    def reemplazar_pagina(self):
        """Simula el reemplazo de página removiendo la página menos
           recientemente utilizada"""
        entrada = self.pagina_lru()

        # Mover la página al disco
        entrada['posicion'] = 'disco'
        del self.marcos_paginas[(entrada['pid'], entrada['indice'])]

    def alojar(self, pid, nuevas_paginas):
        """Aloja la cantidad de páginas dadas para el proceso
           con el pid especificado.
           Retorna
             True  si se pudo asignar la cantidad de páginas
                   solicitada
             False en caso contrario"""
        #print(f'Alojando {pid} {nuevas_paginas}: {self.tabla_paginas}', flush=True)
        if len(self.tabla_paginas) + nuevas_paginas > self.numero_paginas:
            return False

        # Se reservan las entradas en la tabla de páginas
        for i in range(nuevas_paginas):
            self.tabla_paginas[(pid, i)] = {
                    'posicion': 'disco',
                    'pid': pid,
                    'indice': i }

        return True

    def acceder(self, pid, pagina):
        """Simula el acceso a la pagina especificada del proceso dado"""
        #print(f'Accediendo {pid}[{pagina}]', flush=True)
        self.estadisticas['accesos'] += 1
        entrada = self.tabla_paginas[(pid, pagina)]

        if entrada['posicion'] == 'memoria':
            # Mover la página al final de la lista de las menos usadas
            self.marcos_paginas.move_to_end((pid, pagina))
        else:
            # Fallo de página
            self.estadisticas['fallos_pagina'] += 1

            # Si no hay marcos disponibles, se llama al algoritmo de
            # reemplazo de páginas
            if len(self.marcos_paginas) >= self.numero_marcos:
                self.estadisticas['reemplazos'] += 1
                reemplazar_pagina()

            # Actualizar la posición de la página referenciada
            entrada['posicion'] = 'memoria'
            # Tabla de páginas invertida
            self.marcos_paginas[(pid, pagina)] = entrada

    def desalojar(self, pid, cantidad_paginas):
        """Desaloja las páginas alojadas para el proceso con el
           pid especificado"""
        #print(f'Desalojando {pid}: {self.tabla_paginas}', flush=True)

        for i in range(cantidad_paginas):
            entrada = self.tabla_paginas.pop((pid, i))

            if entrada['posicion'] == 'memoria':
                del self.marcos_paginas[(pid, i)]


import struct

pipeRecepcion = "/tmp/pipeRecepcionMemoria"
pipeEnvio     = "/tmp/pipeEnvioMemoria"

TERMINAR  = -1
ALOJAR    =  0
ACCEDER   =  1
DESALOJAR =  2

def ejecutar(planificador):
    abierto = False

    with open(pipeEnvio, 'wb', buffering=0) as envio,\
         open(pipeRecepcion, 'rb', buffering=0) as recepcion:

        def esperarLlamada():
            return struct.unpack("@3i", recepcion.read(12))
        def terminarLlamada():
            envio.write(struct.pack("@i", 1))

        while True:
            tipo, pid, pag = esperarLlamada()

            if tipo == TERMINAR:
                terminarLlamada()
                break
            else:
                if   tipo == ALOJAR:
                    manejador.alojar(pid, pag)
                elif tipo == ACCEDER:
                    manejador.acceder(pid, pag)
                    pass
                elif tipo == DESALOJAR:
                    manejador.desalojar(pid, pag)

                terminarLlamada()

        with open(f'estadisticas-memoria-{planificador}.txt', 'w') as f:
            f.write(f"""\
Total de accesos:    {manejador.estadisticas['accesos']}
Total de reemplazos: {manejador.estadisticas['reemplazos']}
Fallos de página:    {manejador.estadisticas['fallos_pagina']}""")

if __name__ == '__main__':
    if len(argv) != 4:
        print("Uso: python3 memoria-lru.py <identificador>"
                " <numero-paginas> <numero-marcos>")
        exit(-1)

    # Leer parámetros
    planificador = argv[1]
    paginas, marcos = [int(s) for s in argv[2:]]

    manejador = LRU(paginas, marcos)
    ejecutar(planificador)

