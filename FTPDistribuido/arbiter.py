from arbiter.actions import Statuses, NotifyStatusChange
from arbiter.requests import processRequest
from arbiter.state import State, TransientState
from arbiter.health_check import health_checker

from actor import runActor
from asyncServer import server, Request
from utils import loadFromFile, writeToFile

import asyncio
from argparse import ArgumentParser
import logging


def processMessage(message, state):
    if isinstance(message, Request):
        return processRequest(message.message, state, message)
    elif isinstance(message, Statuses):
        return [], NotifyStatusChange(message)
    else:
        raise ValueError()


def runActionWithConfig(config):
    def runAction(action, state, message):
        nonlocal config
        return action.run(state, message, config)

    return runAction


def commit(state):
    writeToFile(config.stateFile, state)


async def main(config):
    topologyQueue = asyncio.Queue()
    initialState = loadFromFile(
        config.stateFile, State(0, [], TransientState(topologyQueue))
    )
    initialState.transient.topologyUpdate = topologyQueue

    await runActor(
        [
            server(config.port),
            health_checker(
                initialState,
                topologyQueue,
                config.healthCheckInterval,
                config.healthCheckTimeout,
            ),
        ],
        initialState,
        processMessage,
        runActionWithConfig(config),
        commit,
    )


if __name__ == "__main__":
    args = ArgumentParser()
    args.add_argument(
        "--port",
        type=int,
        default=4000,
        help="El puerto en el cual ejecutar el servidor",
    )
    args.add_argument(
        "--stateFile",
        default="arbiter.state",
        help="El nombre del archivo en el que se guardará el estado del servidor",
    )
    args.add_argument(
        "--healthCheckInterval",
        type=int,
        default=1,
        help="El intervalo en segundos que se espera para verificar la salud de los servicios",
    )
    args.add_argument(
        "--healthCheckTimeout",
        type=int,
        default=0.5,
        help="El intervalo de tiempo en segundos que tiene un servicio para notificar que es saludable",
    )
    args.add_argument(
        "--logging",
        type=lambda x: getattr(logging, x.upper()),
        default="WARNING",
        help="El nivel de los mensajes de depuración a registrar",
    )

    config = args.parse_args()

    logging.basicConfig(filename="arbiter.log", filemode="w", level=logging.DEBUG)
    print(config.logging)

    loop = asyncio.get_event_loop()
    loop.run_until_complete(main(config))
