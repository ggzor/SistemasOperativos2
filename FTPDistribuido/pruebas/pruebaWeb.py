from PyQt5.QtCore import QUrl, QDir
from PyQt5.QtWidgets import QApplication, QMainWindow, QFrame
from PyQt5.QtWebEngineWidgets import QWebEnginePage, QWebEngineView

from asyncqt import QEventLoop, QThreadExecutor
from aiostream.stream import merge

import asyncio
import time
import json


# Poco importante, sólo algunos protocolos y ya
class Page(QWebEnginePage):
    def __init__(self, parent):
        super().__init__(parent)
        self.queue = asyncio.Queue()

    def javaScriptConsoleMessage(self, level, message, line, src):
        try:
            message = json.loads(message)
            if message["type"] == "message":
                self.queue.put_nowait(message["message"])
            else:
                raise ValueError
        except:
            print(f"js> {'/'.join(src.split('/')[-2:])}:{line} - {message}")

    # GENERADOR 1: JS
    # Quizá lo más importante de esta clase, es un generador asíncrono
    # que produce mensajes enviados desde JavaScript
    async def javaScriptMessages(self):
        while True:
            yield {"type": "js", "payload": await self.queue.get()}
            self.queue.task_done()

    def sendJavaScriptMessage(self, message):
        fut = asyncio.get_event_loop().create_future()
        self.runJavaScript(f"receive({message})", fut.set_result)
        return fut


# Cosas de PyQT
app = QApplication([])
loop = QEventLoop(app)
asyncio.set_event_loop(loop)

uiPath = QUrl.fromLocalFile(QDir.current().filePath("index.html"))
webEngine = QWebEngineView()
page = Page(webEngine)
webEngine.setPage(page)
webEngine.load(uiPath)
window = QMainWindow()
window.setCentralWidget(webEngine)
window.resize(400, 400)
window.show()

# IMPORTANTE: La lógica de procesamiento de datos
# Este es el código interesante, las reglas se pueden expresar claramente
def processMessage(count, message):
    if message == "reset":
        return Reset
    elif count > 0 and message == "remove":
        return RemoveOne
    elif count < 28:
        if message == "add":
            return AddOne("user")
        elif message == "tick":
            return AddOne("system")
        elif message == "server":
            return AddFromServer
    elif message == "server":
        return FailedAddFromServer


# Las respuestas posibles
# Básicamente son funciones que toman el estado actual y devuelven un par
# (nuevoEstado, acciones)
Reset = lambda count: (0, [SendMessage({"type": "reset"})])
RemoveOne = lambda count: (count - 1, [SendMessage({"type": "remove"})])
AddOne = lambda kind: lambda count: (
    count + 1,
    [SendMessage({"type": "add", "kind": kind})],
)
AddFromServer = lambda count: (
    count + 1,
    [SendMessage({"type": "add", "kind": "server"}), AnswerClient(1)],
)
FailedAddFromServer = lambda count: (count, [AnswerClient(0)])

# Las acciones que se pueden ejecutar
from dataclasses import dataclass

# Enviar un mensaje a JS
@dataclass
class SendMessage:
    message: dict


# Contestar al cliente
@dataclass
class AnswerClient:
    status: int


# GENERADOR 2: Es un reloj nada más que da un tick cada segundo,
# pero está expresado como un generador asíncrono
async def clock():
    while True:
        await asyncio.sleep(1)
        yield {"type": "tick", "payload": "tick"}


import sys

# GENERADOR 3: Este generador recibe las peticiones de incremento
# de clientes a través de sockets
async def server():
    queue = asyncio.Queue()

    async def accept(reader, writer):
        try:
            while True:
                await reader.read(1)
                fut = asyncio.get_event_loop().create_future()
                queue.put_nowait(
                    {"type": "server", "payload": "server", "callback": fut.set_result}
                )
                writer.write(int.to_bytes(await fut, 1, sys.byteorder))
                await writer.drain()
        except:
            pass

    await asyncio.start_server(accept, port=1234)

    while True:
        yield await queue.get()
        queue.task_done()


async def main():
    # El estado del programa es sólo una variable
    count = 0
    # Aquí se unen los 3 generadores y se va emitiendo cada evento
    # a como va llegando de cualquiera de los 3 generadores
    eventQueue = merge(page.javaScriptMessages(), clock(), server())

    async with eventQueue.stream() as stream:
        async for message in stream:
            print("In Python:", message)

            result = processMessage(count, message["payload"])

            if result != None:
                # Obtener el nuevo estado y las acciones a ejecutar
                count, actions = result(count)

                # La ejecución de acciones
                for action in actions:
                    if isinstance(action, SendMessage):
                        action.message["count"] = count
                        page.sendJavaScriptMessage(json.dumps(action.message))
                    elif isinstance(action, AnswerClient):
                        message["callback"](action.status)


try:
    with loop:
        loop.run_until_complete(main())
except:
    # Salen varios errores al terminar el programa, pero
    # Básicamente son por el asyncio, nada importante
    pass

# Si sale un error que no fue por el asyncio, usar lo siguiente
# with loop:
#     loop.run_until_complete(main())
