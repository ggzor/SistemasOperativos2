import asyncio
import threading
from functools import singledispatch

import comunicacion_async
from comandos_locales import ejecutar_comando, imprimir_resultado
from comandos_remotos import ejecutar_remoto
from lectura_comandos import (ComandoInvalido, ComandoSalir, leer_comando,
                              leer_configuracion)
from tipos import *


async def ejecutar_servidor(config):
    async def manejar_cliente(reader, writer):
        comando = await comunicacion_async.recv_packet(reader)
        await ejecutar_remoto(comando, reader, writer, config)

    server = await asyncio.start_server(
        manejar_cliente, host=config["host"], port=config["port"]
    )

    async with server:
        await server.serve_forever()


def preparar_servidor(config):
    asyncio.run(ejecutar_servidor(config))


async def evaluar(comando, config):
    comando = leer_comando(comando, config)

    if isinstance(comando, ComandoSalir):
        print("Saliendo...")
        exit(0)
    elif isinstance(comando, ComandoInvalido):
        print(f"Comando inválido: {comando.razon}")
        return

    resultado = await ejecutar_comando(comando, config)
    imprimir_resultado(resultado, config)


async def main():
    config = leer_configuracion()

    if config["interactivo"]:
        # Iniciar servidor de archivos
        print(f"Servidor de archivos iniciado en {obtener_direccion()}")
        config["tarea_servidor"] = threading.Thread(
            target=preparar_servidor, args=(config,), daemon=True
        )
        config["tarea_servidor"].start()

        try:
            while True:
                print("> ", end="")
                entrada = [x.strip() for x in input().split(" ") if x.strip()]
                await evaluar(entrada, config)
        except (EOFError, KeyboardInterrupt):
            # Atrapar errores de interrupciones o fin de entrada
            pass
    else:
        await evaluar(config["comando"], config)


def obtener_direccion():
    import socket

    direccion = "127.0.0.1"
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 53))
        direccion = s.getsockname()[0]
        s.close()
    except:
        pass
    return direccion


if __name__ == "__main__":
    asyncio.run(main())
