import uuid
from dataclasses import dataclass
from functools import singledispatch
from typing import Dict, List

from lenses import lens

from arbiter.actions import PositionAssigned, Statuses, Topology
from asyncServer import Ack, Nack
from Page import UIMessage

from .actions import *
from .state import (
    FileSystem,
    State,
    TransactionCommitted,
    TransactionRejected,
    TransactionRequest,
)


@singledispatch
def processRequest(request, state):
    return [], []


@processRequest.register
def _(request: PositionAssigned, state):
    def mutateState(state: State):
        state.phase = "named"
        state.position = request.position

    if state.phase == "initial":
        return MutateState(mutateState), []


@processRequest.register
def _(request: Topology, state):
    return TopologyUpdate(request), []


@processRequest.register
def _(request: Statuses, state):
    def mutateState(state: State):
        for client in request.statuses:
            state.status[client.name] = client.state

    return MutateState(mutateState), []


@dataclass
class ReplicaUpdate:
    pass


@dataclass
class FileWatchUpdate:
    updates: Dict[Path, File]


@processRequest.register
def _(request: FileWatchUpdate, state: State):
    def mutateState(state: State):
        computer = state.find_computer(state.name)

        if computer.fileSystem == None:
            computer.fileSystem = FileSystem(request.updates)
        else:
            computer.fileSystem.files.update(request.updates)

    return MutateState(mutateState), NotifyFileSystemUpdate(state.name)


fileSystemModifiers = [
    AggregateTransaction,
    DeleteTransaction,
    DeliverTransaction,
    ReceiveTransaction,
    ReplicaUpdate,
    FileWatchUpdate,
]


@processRequest.register(DeleteTransaction)
@processRequest.register(DeliverTransaction)
@processRequest.register(ReceiveTransaction)
@processRequest.register(AggregateTransaction)
def _(request, state: State):
    if state.phase == "running":
        if request.target in state.accessible_computers:
            return (
                [Suspend(fileSystemModifiers), CommitTo(request.target, request),],
                [],
            )
        else:
            return Reject(request.id), []


@processRequest.register
def _(ack: Ack, state: State):
    return (
        [Commited(ack.id), Enable(fileSystemModifiers)],
        [],
    )


@processRequest.register
def _(nack: Nack, state: State):
    return [Rollback(nack.id), Enable(fileSystemModifiers)], []


def recoverFileData(name: str, computer: ComputerData):
    name, data = [
        item for item in computer.fileSystem.files.items() if str(item[1].path) == name
    ][0]

    folderPath = Path(computer.folder)
    return (
        name,
        computer.name,
        data.path,
        data.path.relative_to(folderPath),
        data.size,
    )


@processRequest.register
def _(message: UIMessage, state: State):
    transactionId = uuid.uuid4().hex[:8]

    messageType = message.content["type"]

    if messageType == "send":
        destination = message.content["target"]
        toReceive = []
        transactions = []

        for src, items in message.content["items"].items():
            keys = [uuid.uuid4().hex for _ in range(len(items))]

            transactions.append(
                DeliverTransaction(
                    transactionId,
                    src,
                    destination,
                    [(key, item) for key, item in zip(keys, items)],
                )
            )
            toReceive.extend(
                FileData(*recoverFileData(f, state.find_computer(src)), key)
                for key, f in zip(keys, items)
            )

        tx = TransactionRequest(
            transactionId,
            [*transactions, ReceiveTransaction(transactionId, destination, toReceive),],
        )

        return RunTransaction(tx), []
    elif messageType == "delete":
        transactions = []

        for target, items in message.content["items"].items():
            keys = [uuid.uuid4().hex for _ in range(len(items))]

            transactions.append(
                DeleteTransaction(transactionId, target, list(zip(keys, items)))
            )

        tx = TransactionRequest(transactionId, transactions)

        return RunTransaction(tx), []


@processRequest.register
def _(message: TransactionCommitted, state: State):
    print("COMMITED ALL")
    pass


@processRequest.register
def _(message: TransactionRejected, state: State):
    print("REJECTED ALL")
    pass


@dataclass
class SubscribeFileSystem:
    name: str


@processRequest.register
def _(message: SubscribeFileSystem, state: State):
    if message.name in state.accessible_computers:
        return StreamFileSystemFrom(message.name), []


@dataclass
class RemoteFSUpdate:
    name: str
    fileSystem: FileSystem


@processRequest.register
def _(update: RemoteFSUpdate, state: State):
    return UpdateFS(update.name, update.fileSystem), []


@dataclass
class GetFS:
    name: str


@processRequest.register
def _(request: GetFS, state: State):
    if request.name in state.accessible_computers:
        return (
            AnswerRequestWith(state.find_computer(request.name).fileSystem),
            [],
        )


@dataclass
class PullComplete:
    dest: str
    destPath: Path
    src: str
    srcPath: Path
    key: str


@processRequest.register
def _(request: PullComplete, state: State):
    def mutateState(state: State):
        computer = state.find_computer(request.dest)
        localFile = computer.find_file(str(request.destPath))

        localFile.state = "ready"
        localFile.metadata = None

    return MutateState(mutateState), []


@dataclass
class SendComplete:
    dest: str
    destPath: Path
    src: str
    srcPath: Path
    key: str


@processRequest.register
def _(request: SendComplete, state: State):
    def mutateState(state: State):
        computer = state.find_computer(request.src)
        localFile = computer.find_file(str(request.srcPath))

        if request.key in localFile.uploads:
            localFile.uploads.remove(request.key)

    return MutateState(mutateState), []


@processRequest.register
def _(req: ForceFSUpdate, state: State):
    return req, []
