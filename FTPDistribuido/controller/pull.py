from .state import State
from .requests import PullComplete, SendComplete

import asyncio
import random
from pathlib import Path
import traceback
from asyncServer import server, Response, Stream
from asyncClient import connectTo
from dataclasses import dataclass, astuple

from utils import readObject, writeObject, readObjectSync

from asyncqt import QThreadExecutor


@dataclass
class PullFile:
    dest: str
    destPath: Path
    src: str
    srcPath: Path
    key: str


@dataclass
class Proceed:
    pass


@dataclass
class EOF:
    pass


@dataclass
class FileNotFound:
    path: Path


def available_paths(state: State):
    paths = {}

    for computerName in state.accessible_computers:
        c = state.find_computer(computerName)
        if c.fileSystem:
            for _, fileData in c.fileSystem.files.items():
                if fileData.state == "ready":
                    paths[fileData.path] = (
                        Path(
                            "."
                            if state.name == c.name
                            else f"__replica_{state.name}_{c.name}"
                        )
                        / fileData.path
                    )

    return paths


async def streamFile(localPath, message: PullFile, chunkSize=4096):
    yield Proceed()

    with open(localPath, "rb") as f:
        while True:
            data = f.read(chunkSize)

            yield data

            if not data:
                break

    yield EOF()


async def pullServer(state: State, config):
    async for request in server(config.pullServerPort, ignore_failures=True):
        paths = available_paths(state)

        if isinstance(request.message, PullFile):
            if request.message.srcPath in paths:
                request.respond(
                    Stream(streamFile(paths[request.message.srcPath], request.message))
                )
            else:
                request.respond(Response(FileNotFound(request.message.srcPath)))
        elif isinstance(request.message, PullComplete):
            yield SendComplete(*astuple(request.message))


def remaining_files(state: State):
    remaining = []

    for computerName in state.accessible_computers:
        c = state.find_computer(computerName)
        if c.fileSystem:
            for _, fileData in c.fileSystem.files.items():
                if fileData.state == "pending" and fileData.metadata != None:
                    remaining.append(
                        (
                            Path(
                                "."
                                if state.name == c.name
                                else f"__replica_{state.name}_{c.name}"
                            )
                            / fileData.path,
                            PullFile(
                                c.name,
                                fileData.path,
                                fileData.metadata.src,
                                fileData.metadata.path,
                                fileData.metadata.key,
                            ),
                        )
                    )

    # Para no siempre elegir el mismo
    random.shuffle(remaining)

    return remaining


async def tryPull(item: PullFile, connections, localPath: Path, config):
    # Intentar original
    conn = await connectTo(*connections[0])

    # Intentar con réplica
    if conn == None:
        conn = await connectTo(*connections[1])

    # Ambos fallaron
    if conn == None:
        return None

    try:
        reader, writer = conn

        await writeObject(writer, item)
        response = await readObject(reader)

        if isinstance(response, Proceed):
            localPath.parent.mkdir(parents=True, exist_ok=True)
            with open(localPath, "wb") as f:
                while True:
                    data = await readObject(reader)

                    if isinstance(data, EOF):
                        print("EOF")
                        break

                    f.write(data)
                    print("DATA", data)

            response = PullComplete(*astuple(item))
            await writeObject(writer, response)
            return response
        else:
            return None
    except Exception as ex:
        return None
    finally:
        conn[1].close()
        await conn[1].wait_closed()


async def pullClient(state: State, config):
    while True:
        remaining = remaining_files(state)

        pulled = False
        if len(remaining) > 0:
            localPath, item = remaining[0]

            connections = get_possible_connections(item.dest, state)
            result = await tryPull(item, connections, localPath, config)

            if result != None:
                yield result

        await asyncio.sleep(1)


def get_possible_connections(name: str, state: State):
    current = state.find_computer(name)
    replica = state.find_computer(current.replicatedBy)

    return [
        (current.address, current.pullServerPort),
        (replica.address, replica.pullServerPort),
    ]
