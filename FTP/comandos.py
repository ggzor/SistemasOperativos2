import asyncio
import os
from dataclasses import dataclass
from functools import singledispatch
from typing import List

import comunicacion_async


# Tipos de datos
@dataclass
class InfoArchivo:
    nombre: str
    tamano: int


@dataclass
class ListaArchivos:
    archivos: List[InfoArchivo]


@dataclass
class Continuar:
    razon: object
    datos = None


# Comandos disponibles
@dataclass
class Listar:
    pass


@dataclass
class Descargar:
    ruta: str


@dataclass
class Subir:
    archivo: InfoArchivo
    sobrescribir: bool


# Errores
@dataclass
class NoEncontrado:
    nombre: str


@dataclass
class NoAccesible:
    nombre: str


@dataclass
class RutaInvalida:
    nombre: str


@dataclass
class ErrorConexion:
    ip: str


@dataclass
class SobrescrituraInvalida:
    nombre: str


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
                    await comunicacion_async.send_packet(conexion, respuesta_aceptacion)
                    # Enviar contenido de archivo
                    conexion.write(file.read())
                    await conexion.drain()
                    return
            except PermissionError:
                respuesta = NoAccesible(comando.ruta)
        else:
            respuesta = RutaInvalida(comando.ruta)
    else:
        respuesta = NoEncontrado(comando.ruta)

    # Enviar respuesta
    await comunicacion_async.send_packet(conexion, respuesta)


@ejecutar_remoto.register
async def _(comando: Subir, reader, writer, config):
    """Comando remoto para subir un archivo"""
    archivo_destino = os.path.join(config["directorio"], comando.archivo.nombre)

    if not os.path.exists(archivo_destino) or comando.sobrescribir:
        try:
            with open(archivo_destino, "wb") as archivo:
                # Enviar mensaje de aceptacion
                respuesta_aceptacion = Continuar(comando, None)
                await comunicacion_async.send_packet(conexion, respuesta_aceptacion)
                # Recibir contenido de archivo a escribir
                archivo.write(await conexion.readexactly(comando.archivo.tamano))
                return
        except PermissionError:
            respuesta = NoAccesible(comando.archivo.nombre)
    else:
        respuesta = SobrescrituraInvalida(comando.archivo)

    # Enviar respuesta
    await comunicacion_async.send_packet(conexion, respuesta)
