import asyncio
from functools import singledispatch
from pathlib import Path

import comunicacion_async
from tipos import *


async def ejecutar_comando(comando, config):
    destinos, comando = comando

    if len(destinos) == 1:
        return await conectar(destinos[0], config["port"], comando, config)
    elif len(destinos) > 1:
        if isinstance(comando, Subir):
            resultados = await asyncio.gather(
                *[conectar(h, config["port"], comando, config) for h in destinos]
            )

            for host, resultado in zip(destinos, resultados):
                print(f"[{host}] ", end="")
                if resultado == None:
                    print("Completado exitosamente")
                else:
                    imprimir_resultado(resultado, config)
        else:
            return OperacionInvalida("Este comando no soporta múltiples direcciones")


async def conectar(host, puerto, comando, config):
    try:
        reader, writer = await asyncio.open_connection(host, puerto)
        return await procesar_comando(comando, reader, writer, host, config)
    except (ConnectionRefusedError, OSError):
        return ErrorConexion(host)


@singledispatch
async def procesar_comando(comando, reader, writer, host, config):
    await comunicacion_async.send_packet(writer, comando)
    return await comunicacion_async.recv_packet(reader)


@procesar_comando.register
async def _(comando: Descargar, reader, writer, host, config):
    await comunicacion_async.send_packet(writer, comando)
    respuesta = await comunicacion_async.recv_packet(reader)

    if isinstance(respuesta, Continuar):
        print(
            f"Descargando archivo {respuesta.datos.nombre} ({respuesta.datos.tamano} bytes)"
            f" desde {host}"
        )

        archivo = Path(config["directorio"], respuesta.datos.nombre)
        with open(str(archivo), "wb") as f:
            f.write(await reader.readexactly(respuesta.datos.tamano))

        print(f"Descarga completa desde {host}")
    else:
        return respuesta


@procesar_comando.register
async def _(comando: Subir, reader, writer, host, config):
    await comunicacion_async.send_packet(writer, comando)
    respuesta = await comunicacion_async.recv_packet(reader)

    if isinstance(respuesta, Continuar):
        print(
            f"Subiendo archivo {comando.archivo.nombre} ({comando.archivo.tamano} bytes)"
            f" a {host}"
        )

        archivo = Path(config["directorio"], comando.archivo.nombre)
        with open(archivo, "rb") as f:
            writer.write(f.read())
            await writer.drain()
        print(f"Carga completa a {host}")
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
        elif isinstance(resultado, OperacionInvalida):
            print(f"La operación no es válida: {resultado.razon}")
        else:
            raise NotImplementedError("Respuesta inesperada", resultado)


@imprimir_resultado.register
def _(resultado: ListaArchivos, config):
    print(f"Existen {len(resultado.archivos)} archivos:")
    print("\n".join(f"  - {x.nombre}" for x in resultado.archivos))
