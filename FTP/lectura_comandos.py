import argparse
import os
from dataclasses import dataclass
from pathlib import Path

from tipos import *


@dataclass
class ComandoInvalido:
    razon: str


@dataclass
class ComandoSalir:
    pass


def es_ip_valida(ip):
    componentes = ip.split(".")

    def numero_valido(numero):
        try:
            valor = int(numero)
            return str(valor) == numero and 0 <= valor <= 255
        except:
            return False

    return len(componentes) == 4 and all(map(numero_valido, componentes))


def leer_comando(comando, config):
    if len(comando) > 0 and comando[0] == "salir":
        return ComandoSalir()

    if "$" not in comando:
        return ComandoInvalido(
            "No se encontró el caracter '$' en la expresión. "
            "Este divide los destinos del comando."
        )

    # Dividir destinos de comando
    indice = comando.index("$")
    parametros = comando[:indice]
    destinos = comando[indice + 1 :]

    # Validar longitudes de listas
    if len(parametros) == 0:
        return ComandoInvalido(
            "No se proporcionó un comando. "
            "Los comandos válidos son: listar, descargar y subir"
        )

    if len(destinos) == 0:
        return ComandoInvalido(
            "No se proporcionaron los destinos. "
            "Los destinos son direcciones IPv4 separadas por espacios."
        )

    # Validar destinos
    for d in destinos:
        if not es_ip_valida(d):
            return ComandoInvalido(f"La IP {d} no es válida.")

    # Determinar comando
    nombre = parametros[0]
    if nombre == "listar":
        if len(destinos) != 1:
            return ComandoInvalido(
                "El comando 'listar' sólo toma un destino como parámetro"
            )

        if len(parametros) != 1:
            return ComandoInvalido(
                "El comando listar no toma parámetros aparte de los destinos"
            )

        comando = Listar()
    elif nombre == "descargar":
        if len(parametros) != 2:
            return ComandoInvalido(
                "Se requiere exactamente un parámetro, "
                "que es la ruta del archivo a descargar."
            )

        if len(destinos) != 1:
            return ComandoInvalido("Aún no se permite descargar de múltiples fuentes")

        comando = Descargar(parametros[1])
    elif nombre == "subir":
        if len(parametros) not in [2, 3]:
            return ComandoInvalido(
                "El comando recibe un parámetro obligatorio (ruta del archivo a subir) "
                "y uno opcional ('-s') que indica si se debe sobrescribir el archivo "
                "si ya se encuentra en el servidor."
            )

        # Revisar si se debe sobrescribir
        sobrescribir = False
        if len(parametros) == 3:
            if parametros[2] == "-s":
                sobrescribir = True
            else:
                return ComandoInvalido("El parámetro opcional sólo puede ser '-s'")

        # Comprobar validez del archivo
        archivo = Path(config["directorio"], parametros[1])
        if not archivo.exists():
            return ComandoInvalido(f"No existe el archivo especificado: {archivo}")

        if not archivo.is_file():
            return ComandoInvalido(f"Sólo se pueden subir archivos: {archivo}")

        comando = Subir(
            InfoArchivo(str(archivo), os.path.getsize(str(archivo))), sobrescribir
        )
    else:
        return ComandoInvalido(f"No existe el comando {nombre}")

    return (destinos, comando)


def leer_configuracion():
    from sys import argv

    config = {"interactivo": "--" not in argv}

    if config["interactivo"]:
        opciones = argv[1:]
    else:
        indice = argv.index("--")
        opciones = argv[1:indice]
        config["comando"] = argv[indice + 1 :]

    parser = argparse.ArgumentParser(description="Servidor FTP")
    parser.add_argument(
        "--directorio",
        default="./",
        help="El directorio desde el que se enviaran y recibiran los archivos",
    )
    parser.add_argument(
        "--host",
        default="0.0.0.0",
        help="La red en la que se debe ejecutar el programa",
    )
    parser.add_argument(
        "--port", default=8888, help="El puerto en el que se harán las conexiones"
    )
    opciones = parser.parse_args(opciones)

    return {**config, **vars(opciones)}
