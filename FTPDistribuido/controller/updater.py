from .state import State
from .actions import ForceFSUpdate

from asyncClient import connectTo
from utils import readObject, writeObject
import asyncio
import logging
import traceback


async def forcedUpdate(queue, state: State):
    while True:
        name, update = await queue.get()
        computer = state.find_computer(name)

        conn = await connectTo(computer.address, computer.requestsPort)

        if conn != None:
            try:
                await writeObject(conn[1], ForceFSUpdate(update))

                conn[1].close()
                await conn[1].wait_closed()
            except Exception as ex:
                print(ex)
                traceback.print_tb(ex.__traceback__)

        if False:
            yield None
