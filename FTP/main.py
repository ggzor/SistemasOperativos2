import asyncio
import threading
from functools import singledispatch

import comunicacion_async
from comandos import *
from lectura_comandos import (ComandoInvalido, ComandoSalir, leer_comando,
                              leer_configuracion)


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


async def ejecutar_comando(comando, config):
    destinos, comando = comando
    try:
        reader, writer = await asyncio.open_connection(destinos[0], config["port"])
        return await procesar_comando(comando, reader, writer)
    except ConnectionRefusedError:
        return ErrorConexion(destinos[0])


@singledispatch
async def procesar_comando(comando, reader, writer):
    await comunicacion_async.send_packet(writer, comando)
    return await comunicacion_async.recv_packet(reader)


@procesar_comando.register
async def _(comando: Descargar, reader, writer):
    await comunicacion_async.send_packet(writer, comando)
    respuesta = await comunicacion_async.recv_packet(reader)

    if isinstance(respuesta, Continuar):
        print(
            f"Descargando archivo {respuesta.datos.nombre} ({respuesta.datos.tamano} bytes)..."
        )
        with open(respuesta.datos.nombre, "wb") as f:
            f.write(await reader.readexactly(respuesta.datos.tamano))
        print("Archivo descargado.")
    else:
        return respuesta


@procesar_comando.register
async def _(comando: Subir, reader, writer):
    await comunicacion_async.send_packet(writer, comando)
    respuesta = await comunicacion_async.recv_packet(reader)

    if isinstance(respuesta, Continuar):
        print(
            f"Subiendo archivo {comando.archivo.nombre} ({comando.archivo.tamano} bytes)..."
        )
        with open(comando.archivo.nombre, "rb") as f:
            writer.write(f.read())
            await writer.drain()
        print("Archivo subido.")
    else:
        return respuesta


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


@singledispatch
def imprimir_resultado(resultado, config):
    if resultado != None:
        if isinstance(resultado, NoEncontrado):
            print(f"No se encontró el archivo: {resultado.nombre}")
        elif isinstance(resultado, NoAccesible):
            print(f"No se pudo acceder al archivo: {resultado.nombre}")
        elif isinstance(resultado, RutaInvalida):
            print(f"La ruta dada no es válida: {resultado.nombre}")
        elif isinstance(resultado, SobrescrituraInvalida):
            print(
                f"No se proporcionó la opción de sobrescribir el archivo: {resultado.nombre}"
            )
        elif isinstance(resultado, ErrorConexion):
            print(f"No se pudo conectar: {resultado.ip}")
        else:
            raise NotImplementedError("Respuesta inesperada", resultado)


@imprimir_resultado.register
def _(resultado: ListaArchivos, config):
    print(f"Existen {len(resultado.archivos)} archivos:")
    print("\n".join(f"  - {x.nombre}" for x in resultado.archivos))


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
