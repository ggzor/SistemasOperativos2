from .state import State, ComputerData
from .requests import RemoteFSUpdate, SubscribeFileSystem, GetFS

from asyncClient import connectTo, callRemote
from utils import readObject, writeObject

import aiostream

import asyncio
import logging
import traceback
from dataclasses import dataclass

watchFSLogger = logging.getLogger("watchFSLogger")


async def getFSFrom(name, host, port, timeout):
    try:
        return await asyncio.wait_for(callRemote(GetFS(name), host, port), timeout)
    except asyncio.TimeoutError:
        return None


async def pure(x):
    return x


async def tryGetFS(computer: ComputerData, state: State, config):
    replicaComputer = state.find_computer(computer.replicatedBy)

    original, replica = await asyncio.gather(
        getFSFrom(
            computer.name,
            computer.address,
            computer.requestsPort,
            config.fsUpdateTimeout,
        ),
        pure(computer.fileSystem)
        if replicaComputer.name == state.name
        else getFSFrom(
            computer.name,
            replicaComputer.address,
            replicaComputer.requestsPort,
            config.fsUpdateTimeout,
        ),
    )

    return computer.name, original or replica


async def watchRemoteFS(state: State, config):
    while True:
        if state.phase == "running":
            to_watch = [c for c in state.computers if c.name != state.name]

            if to_watch:
                results = await asyncio.gather(
                    *map(lambda c: tryGetFS(c, state, config), to_watch)
                )

                for name, fs in results:
                    if state.find_computer(name).fileSystem != fs:
                        yield RemoteFSUpdate(name, fs)

        await asyncio.sleep(1)
