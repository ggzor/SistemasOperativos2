from .state import State
from .requests import _

import asyncio

from asyncqt import QThreadExecutor


async def pull_service(state: State):
    with QThreadExecutor(1) as executor:
        while True:
            result = await asyncio.get_event_loop().run_in_executor(
                executor, nextRequest, state
            )

            if result != None:
                print("Algo se ejecutó en otro hilo", result)
                yield


def available_paths(state: State):
    return [str(f.path) for f in state.current_computer.fileSystem.files.values()]


def nextRequest(state: State):
    import time

    time.sleep(2)

    return "OK"
