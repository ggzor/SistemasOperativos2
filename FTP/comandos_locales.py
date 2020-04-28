import asyncio
from functools import singledispatch

import comunicacion_async
from tipos import *


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
