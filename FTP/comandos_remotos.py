import asyncio
import os
from functools import singledispatch

import comunicacion_async
from tipos import *


# Comandos
@singledispatch
async def ejecutar_remoto(comando, reader, writer, config):
    raise NotImplementedError("No se reconoce el comando especificado")


@ejecutar_remoto.register
async def _(comando: Listar, reader, writer, config):
    """Comando remoto para listar los archivos que se encuentran en el directorio de trabajo"""
    info_archivos = [
        InfoArchivo(f, os.path.getsize(os.path.join(config["directorio"], f)))
        for f in os.listdir(config["directorio"])
    ]
    respuesta = ListaArchivos(info_archivos)
    await comunicacion_async.send_packet(writer, respuesta)


@ejecutar_remoto.register
async def _(comando: Descargar, reader, writer, config):
    """Comando remoto para descargar un archivo local"""
    archivo_destino = os.path.join(config["directorio"], comando.ruta)
    respuesta = None

    if os.path.exists(archivo_destino):
        if os.path.isfile(archivo_destino):
            try:
                with open(archivo_destino, "rb") as file:
                    # El archivo fue encontrado y es accesible
                    info_archivo = InfoArchivo(
                        comando.ruta, os.path.getsize(archivo_destino)
                    )

                    # Enviar mensaje de aceptacion
                    respuesta_aceptacion = Continuar(comando, info_archivo)
                    await comunicacion_async.send_packet(writer, respuesta_aceptacion)
                    # Enviar contenido de archivo
                    writer.write(file.read())
                    await writer.drain()
                    return
            except PermissionError:
                respuesta = NoAccesible(comando.ruta)
        else:
            respuesta = RutaInvalida(comando.ruta)
    else:
        respuesta = NoEncontrado(comando.ruta)

    # Enviar respuesta
    await comunicacion_async.send_packet(writer, respuesta)


@ejecutar_remoto.register
async def _(comando: Subir, reader, writer, config):
    """Comando remoto para subir un archivo"""
    archivo_destino = os.path.join(config["directorio"], comando.archivo.nombre)

    if not os.path.exists(archivo_destino) or comando.sobrescribir:
        try:
            with open(archivo_destino, "wb") as archivo:
                # Enviar mensaje de aceptacion
                respuesta_aceptacion = Continuar(comando, None)
                await comunicacion_async.send_packet(writer, respuesta_aceptacion)
                # Recibir contenido de archivo a escribir
                archivo.write(await reader.readexactly(comando.archivo.tamano))
                return
        except PermissionError:
            respuesta = NoAccesible(comando.archivo.nombre)
    else:
        respuesta = SobrescrituraInvalida(comando.archivo.nombre)

    # Enviar respuesta
    await comunicacion_async.send_packet(writer, respuesta)
