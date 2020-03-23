import struct

pipeRecepcion = "/tmp/pipeRecepcionMemoria"
pipeEnvio     = "/tmp/pipeEnvioMemoria"

TERMINAR  = -1
ALOJAR    =  0
ACCEDER   =  1
DESALOJAR =  2

def ejecutar():
    abierto = False

    print('Inicializando manejo de memoria...')
    with open(pipeEnvio, 'wb', buffering=0) as envio,\
         open(pipeRecepcion, 'rb', buffering=0) as recepcion:

        def esperarLlamada():
            datos = recepcion.read(12)
            print(len(datos))
            return struct.unpack("@3i", datos)
        def terminarLlamada():
            envio.write(struct.pack("@i", 1))
        
        while True:
            print('Esperando llamada...')
            tipo, pid, pag = esperarLlamada()

            if tipo == TERMINAR:
                terminarLlamada()
                print('Terminando manejo de memoria')
                break
            else:
                if   tipo == ALOJAR:
                    print(f'Alojando pid={pid} pags={pag}')
                elif tipo == ACCEDER:
                    print(f'Accediendo pid={pid} pag={pag}')
                    pass
                elif tipo == DESALOJAR:
                    print(f'Desalojando pid={pid}')

                terminarLlamada()

        print('Escribiendo estadísticas')

if __name__ == '__main__':
    ejecutar()
