from dataclasses import dataclass
from typing import List


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
    datos: object


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
