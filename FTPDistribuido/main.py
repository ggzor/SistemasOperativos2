from mainController import controller
from config import argumentsParser
from Page import Page, UIMessage

import asyncio
import logging
import traceback
import os
import sys

from PyQt5.QtCore import QDir, QUrl
from PyQt5.QtWebEngineWidgets import QWebEnginePage, QWebEngineView
from PyQt5.QtWidgets import QApplication, QMainWindow

from asyncqt import QEventLoop

args = argumentsParser.parse_args()

# logging.basicConfig(filename=args.logFile, filemode="w", level=args.logging)

# Redirección de errores no relevantes
# sys.stderr = open("main.err.log", "w")


uiPath = QUrl.fromLocalFile(QDir.current().filePath("ui/index.html"))

app = QApplication([])
loop = QEventLoop(app)
asyncio.set_event_loop(loop)

webEngine = QWebEngineView()
page = Page(webEngine)
webEngine.setPage(page)
loaded = loop.create_future()
page.loadFinished.connect(lambda: loaded.set_result(1))
webEngine.load(uiPath)
window = QMainWindow()
window.setCentralWidget(webEngine)
window.resize(500, 600)
window.show()


async def main():
    await loaded
    await controller(page.javaScriptMessages(), page.sendJavaScriptMessage, args)


def logException(ex):
    tb = "\n".join(traceback.format_tb(ex.__traceback__))
    logging.error(f"Exception while exiting program: \n{ex}\n{tb}")


try:
    with loop:
        loop.run_until_complete(main())
except Exception as ex:
    logException(ex)
