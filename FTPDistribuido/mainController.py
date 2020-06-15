from controller.committer import commiter
from controller.file_watcher import watchFileSystem
from controller.file_system_update_watcher import watchRemoteFS
from controller.requests import processRequest
from controller.state import State

from arbiter.requests import Register
from arbiter.health_check import healthy_server

from actor import runActor
from asyncClient import client
from asyncServer import server, Request
from utils import loadFromFile, writeToFile

import asyncio
from collections import defaultdict
import logging
import traceback
import uuid

from aiostream.stream import merge

mainLogger = logging.getLogger("main")


async def controller(uiMessages, sendUIMessage, config):
    state = loadFromFile(config.stateFile, default=State("firstTime"))
    state.transactionQueue = asyncio.Queue()

    # Fase de asignación de nombre
    if state.phase == "firstTime":
        state.phase = "initial"
        state.name = uuid.uuid4().hex[:8]
        writeToFile(config.stateFile, state)

    # Deshacer transacciones no guardadas
    while state.transactions:
        _, transaction = state.transactions.popitem()
        transaction.rollback(state)
        writeToFile(config.stateFile, state)

    await sendUIMessage(projectUIState(state, config))

    # Registrar servidor para revisión de salud
    asyncio.create_task(healthy_server(config.healthCheckPort))

    # Flujos
    flujos = [
        uiMessages,
        client(
            config.arbiterHost,
            config.arbiterPort,
            Register(
                state.name, config.requestsPort, config.folder, config.healthCheckPort
            ),
        ),
        server(config.requestsPort),
        commiter(state.transactionQueue, state),
        watchFileSystem(state, config),
        watchRemoteFS(state, config),
    ]

    def processMessage(message, state: State):
        shouldIgnore = type(message) in state.ignoreRequestsOfType or (
            isinstance(message, Request)
            and message.message in state.ignoreRequestsOfType
        )
        if shouldIgnore:
            mainLogger.warning(f"Request ignored explicitly: \n{message}")
            return [], []

        if isinstance(message, Request):
            result = processRequest(message.message, state)
        else:
            result = processRequest(message, state)

        if result == None:
            return [], []
        else:
            return result

    def runAction(action, state, message):
        return action.run(state, message, config)

    def commit(state):
        writeToFile(config.stateFile, state)
        sendUIMessage(projectUIState(state, config))

    await runActor(flujos, state, processMessage, runAction, commit)


def projectUIState(state: State, config):
    if state.phase == "initial":
        return {"phase": "initial", "computers": []}
    elif state.phase == "named":
        return {"phase": "named", "name": state.name, "computers": []}
    elif state.phase == "running":
        return {
            "phase": "running",
            "computers": [
                {
                    "name": f"Computadora {c.position}",
                    "hash": c.name,
                    "fileSystem": getVirtualFS(
                        c.fileSystem
                        and sorted(
                            [(path.parts, f) for path, f in c.fileSystem.files.items()],
                            key=lambda x: x[0],
                        ),
                        c.folder,
                        c.folder,
                    ),
                    "info": {
                        "isLocal": c.isLocal,
                        "status": state.status.get(c.name, "pending"),
                        "replica": c.replicatedBy
                        and [
                            f"Computadora {other.position}"
                            for other in state.computers
                            if other.name == c.replicatedBy
                        ][0],
                        "syncState": {"state": "synced"},  # Check
                        "fileCount": c.fileSystem and len(c.fileSystem.files),
                        "totalFileSize": c.fileSystem
                        and sum(f.size for f in c.fileSystem.files.values()),
                        "sentFiles": c.sentFiles,
                        "receivedFiles": c.receivedFiles,
                    },
                }
                for c in sorted(state.computers, key=lambda c: c.position)
            ],
        }


def getVirtualFS(fs, folder, folderHash):
    if fs == None:
        return None

    files = []
    size = 0
    folders = defaultdict(lambda: [])

    beingDeleted = True
    for p, f in fs:
        if len(p) == 1:
            files.append(
                {
                    "type": "file",
                    "name": f.path.name,
                    "hash": str(f.path),
                    "size": f.size,
                    "state": "deleting" if len(f.deletes) > 0 else f.state,
                    "uploading": len(f.uploads) > 0,
                }
            )
            beingDeleted &= len(f.deletes) > 0
            size += f.size
        else:
            folders[p[0]].append((p[1:], f))

    for directory, items in folders.items():
        result = getVirtualFS(
            sorted(items, key=lambda x: x[0]), directory, folderHash + "/" + directory
        )
        size += result["size"]
        files.append(result)

    return {
        "type": "directory",
        "name": folder,
        "hash": folderHash,
        "size": size,
        "files": files,
        "state": "deleting" if beingDeleted else "ready",
    }
