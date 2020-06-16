import asyncio
import code
import logging
import uuid
import sys
import traceback

from asyncServer import readObject, writeObject, Ack
from arbiter.requests import Register
from arbiter.health_check import healthy_server

from controller.actions import Accept, Reject

asyncClientLogger = logging.getLogger("asyncClient")


async def connectTo(host, port):
    try:
        return await asyncio.open_connection(host, port)
    except Exception as ex:
        return None


async def callRemote(request, host, port):
    try:
        reader, writer = await asyncio.open_connection(host, port)
        await writeObject(writer, request)
        return await readObject(reader)
    except:
        return None


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


async def commiter():
    r1, w1 = await asyncio.open_connection("127.0.0.1", 4001)
    r2, w2 = await asyncio.open_connection("127.0.0.1", 4002)

    transaction = RemoveTransaction("abc", sys.argv[1], [])

    async def makeOperation(r, w):
        try:
            await writeObject(w, transaction)
            response = await readObject(r)
            return response
        except:
            return Reject(transaction.id)

    async def answerOk(w):
        await writeObject(w, Ack())

    succeeded = False
    try:
        result = await asyncio.wait_for(
            asyncio.gather(
                *map(lambda args: makeOperation(*args), [[r1, w1], [r2, w2]])
            ),
            2,
        )

        if all([isinstance(res, Accept) for res in result]):
            print("Commit!!!")
            succeeded = True
            await asyncio.gather(*map(answerOk, [w1, w2]))
    finally:
        if not succeeded:
            print("Rollback!!")

        for w in [w1, w2]:
            w.close()

        await asyncio.gather(*map(lambda w: w.wait_closed(), [w1, w2]))


if __name__ == "__main__":
    asyncio.run(commiter())
