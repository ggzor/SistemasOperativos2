import asyncio
import logging
import pickle
import struct
import traceback
from dataclasses import dataclass
from typing import Any, AsyncIterable, Callable, Optional, Union

from rx import operators as ops
from rx.scheduler.eventloop import AsyncIOScheduler
from rx.core.notification import OnNext, OnError
from rx.core.typing import Observable

from utils import readObject, writeObject

## Tipos para las respuestas

# Un solo objeto como respuesta
@dataclass
class Response:
    response: Any


# Un objeto como respuesta que requiere confirmación
@dataclass
class ResponseWithAck:
    id: str
    response: Any
    timeout: Optional[float]


# Un flujo que no espera confirmacion
@dataclass
class Stream:
    stream: AsyncIterable[Any]


# Un flujo que requiere confirmación
@dataclass
class AcknowledgedStream:
    stream: AsyncIterable[Any]


## Tipo para las solicitudes
@dataclass
class Request:
    address: str
    message: Any
    respond: Callable[[Union[Response, Stream, AcknowledgedStream]], None]


## Tipo para las confirmaciones
@dataclass
class Ack:
    id: str = None


@dataclass
class Nack:
    id: str = None


asyncServerLogger = logging.getLogger("asyncServer")

# Servidor
async def server(port, ignore_failures=False):
    queue = asyncio.Queue()

    async def accept(reader, writer):
        address = writer.get_extra_info("peername")[0]

        try:
            while True:
                message = await readObject(reader)

                responseFut = asyncio.get_event_loop().create_future()
                queue.put_nowait(Request(address, message, responseFut.set_result))
                response = await responseFut

                if isinstance(response, Response):
                    await writeObject(writer, response.response)
                elif isinstance(response, ResponseWithAck):
                    try:

                        async def operation():
                            await writeObject(writer, response.response)
                            return await readObject(reader)

                        if response.timeout is None:
                            answer = await operation()
                        else:
                            answer = await asyncio.wait_for(
                                operation(), response.timeout
                            )
                        writer.close()

                        if isinstance(answer, Ack):
                            queue.put_nowait(Ack(response.id))
                    except:
                        queue.put_nowait(Nack(response.id))
                elif isinstance(response, Stream):
                    async for item in response.stream:
                        await writeObject(writer, item)
                elif isinstance(response, AcknowledgedStream):
                    iterator = response.stream.__aiter__()
                    try:
                        item = await iterator.asend(None)
                        while True:
                            await writeObject(writer, item)
                            acknowledge = await readObject(reader)
                            item = await iterator.asend(isinstance(acknowledge, Ack))
                    except StopAsyncIteration:
                        pass
        except Exception as ex:
            if not ignore_failures:
                tb = "\n".join(traceback.format_tb(ex.__traceback__))
                asyncServerLogger.warning(
                    f"Exception while processing client: {address}\n{ex}\n{tb}"
                )

    server = await asyncio.start_server(accept, port=port)

    async with server:
        while True:
            yield await queue.get()
            queue.task_done()


# Utilerías
async def streamQueue(queue):
    while True:
        yield await queue.get()
        queue.task_done()


async def streamQueueWithAcknowledge(queue, pending, commit):
    isFromQueue = False

    while pending:
        success = yield pending[0]
        if success:
            if isFromQueue:
                queue.task_done()
                isFromQueue = False
            pending.pop(0)
            commit()

        pending.append(await queue.get())
        isFromQueue = True


async def asGenerator(obs):
    queue = asyncio.Queue()

    def on_next(i):
        queue.put_nowait(i)

    disposable = obs.pipe(ops.materialize()).subscribe(
        on_next=on_next, scheduler=AsyncIOScheduler(loop=asyncio.get_event_loop())
    )

    while True:
        i = await queue.get()
        if isinstance(i, OnNext):
            yield i.value
            queue.task_done()
        elif isinstance(i, OnError):
            disposable.dispose()
            print("EXCEPTION IN GEN", i.value)
            break
        else:
            disposable.dispose()
            break
