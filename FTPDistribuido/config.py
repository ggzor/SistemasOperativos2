from argparse import ArgumentParser

import logging

argumentsParser = ArgumentParser()
argumentsParser.add_argument(
    "folder", help="La carpeta desde la cual servir los archivos"
)

argumentsParser.add_argument(
    "--requestsPort",
    default=4001,
    type=int,
    help="El puerto donde se escuchan solicitudes",
)

argumentsParser.add_argument("arbiterHost", help="La dirección del árbitro")
argumentsParser.add_argument(
    "--arbiterPort", default=4000, type=int, help="El puerto donde escucha el árbitro"
)

argumentsParser.add_argument(
    "--healthCheckPort",
    default=4321,
    type=int,
    help="El puerto a donde se atenderán las solicitudes de revisión de salud",
)

argumentsParser.add_argument(
    "--pullServerPort",
    default=50001,
    type=int,
    help="El puerto a donde se escuchan las solicitudes de archivos",
)


argumentsParser.add_argument(
    "--stateFile",
    default="main.state",
    help="El archivo en el que se guardarán los datos del programa",
)

argumentsParser.add_argument(
    "--logFile", default="main.log", help="El archivo a dónde guardar los registros"
)

argumentsParser.add_argument(
    "--logging",
    type=lambda x: getattr(logging, x.upper()),
    default="WARNING",
    help="El nivel de los mensajes de depuración a registrar",
)

argumentsParser.add_argument(
    "--fileWatchInterval",
    type=float,
    default=1,
    help="El intervalo de tiempo en el cuál volver a revisar cambios en el sistema de archivos",
)

argumentsParser.add_argument(
    "--fsUpdateTimeout",
    type=float,
    default=0.200,
    help="El tiempo a esperar para conectarse al servidor principal",
)
