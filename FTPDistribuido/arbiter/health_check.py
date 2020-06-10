from .actions import NodeStatus, Statuses, get_statuses
from .state import Client

from asyncServer import Response, server
from utils import readObject, writeObject

import asyncio
from dataclasses import dataclass


@dataclass
class HealthCheck:
    pass


@dataclass
class Healthy:
    pass


async def healthy_server(port):
    async for request in server(port, ignore_failures=True):
        if isinstance(request.message, HealthCheck):
            request.respond(Response(Healthy()))


async def ask_service(client: Client, timeout):
    try:
        reader, writer = await asyncio.open_connection(
            client.address, client.healthCheckPort
        )

        async def communicate():
            await writeObject(writer, HealthCheck())
            return await readObject(reader)

        response = await asyncio.wait_for(communicate(), timeout)

        writer.close()
        await writer.wait_closed()

        if isinstance(response, Healthy):
            return NodeStatus(client.name, "online")
    except:
        pass

    return NodeStatus(client.name, "offline")


async def health_checker(state, topologyUpdate, interval, timeout):
    prevTask = None
    while True:
        result = Statuses(
            await asyncio.gather(*map(lambda c: ask_service(c, timeout), state.clients))
        )

        if result != get_statuses(state):
            yield result

        waitTime = asyncio.create_task(asyncio.sleep(interval))
        done, pending = await asyncio.wait(
            [waitTime, topologyUpdate.get() if prevTask is None else prevTask],
            return_when=asyncio.FIRST_COMPLETED,
        )

        if waitTime in done:
            prevTask = pending.pop()
        else:
            prevTask = None
