import asyncio
import dataclasses
import json
import logging

from PyQt5.QtWebEngineWidgets import QWebEnginePage


@dataclasses.dataclass
class UIMessage:
    content: dict


logger = logging.getLogger("ui")


class Page(QWebEnginePage):
    def __init__(self, parent):
        super().__init__(parent)
        self.queue = asyncio.Queue()

    def javaScriptConsoleMessage(self, level, message, line, src):
        try:
            jsonMessage = json.loads(message)
            if jsonMessage["type"] == "message":
                self.queue.put_nowait(jsonMessage["message"])
            else:
                raise ValueError
        except:
            logger.info(f"{'/'.join(src.split('/')[-2:])}:{line} - {message}")

    async def javaScriptMessages(self):
        while True:
            yield UIMessage(await self.queue.get())
            self.queue.task_done()

    def sendJavaScriptMessage(self, message):
        fut = asyncio.get_event_loop().create_future()
        self.runJavaScript(f"receive({json.dumps(message)})", fut.set_result)
        return fut
