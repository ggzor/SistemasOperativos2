import asyncio
import code
import logging
import uuid
import sys
import traceback

from asyncServer import readObject, writeObject, Ack
from arbiter.requests import Register
from arbiter.health_check import healthy_server

asyncClientLogger = logging.getLogger("asyncClient")


async def client(host, port, initialRequest, maxDelay=5, initialDelay=0.100):
    nextDelay = initialDelay
    while True:
        try:
            reader, writer = await asyncio.open_connection(host, port)
            nextDelay = initialDelay

            await writeObject(writer, initialRequest)

            while True:
                yield await readObject(reader)

        except asyncio.CancelledError:
            raise
        except Exception as ex:
            tb = "\n".join(traceback.format_tb(ex.__traceback__))

            asyncClientLogger.warning(
                f"Exception while connection:\n{ex}\n{tb}"
                + f"\nReconnecting in {nextDelay}s"
            )

            await asyncio.sleep(nextDelay)

            nextDelay = min(maxDelay, nextDelay * 2)


async def main():
    healthPort = int(sys.argv[1])
    if len(sys.argv) > 2:
        name = sys.argv[2]
    else:
        name = uuid.uuid4().hex[:4]

    health = asyncio.create_task(healthy_server(healthPort))

    async for message in client("127.0.0.1", 4000, Register(name, healthPort)):
        print(message)


if __name__ == "__main__":
    asyncio.run(main())
