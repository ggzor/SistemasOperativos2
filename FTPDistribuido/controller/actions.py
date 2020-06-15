from typing import Any, List, Union

from rx import operators as ops

from arbiter.requests import Node, Topology
from asyncServer import Ack, Request, Response, ResponseWithAck, Stream, asGenerator
from utils import writeToFile

from .state import *


@dataclass
class MutateState:
    mutation: callable

    def run(self, state: State, request, conf):
        self.mutation(state)


def search_client(name, state) -> ComputerData:
    candidates = [c for c in state.computers if c.name == name]

    if candidates:
        return candidates[0]


def computer_data_from(client: Node, isLocal) -> ComputerData:
    return ComputerData(
        client.name,
        client.position,
        client.address,
        client.requestsPort,
        client.folder,
        client.replicatedBy,
        client.replicating,
        isLocal,
    )


@dataclass
class TopologyUpdate:
    topology: Topology

    def run(self, state: State, request, conf):
        if state.phase == "named":
            state.phase = "running"
            state.computers = [
                computer_data_from(c, isLocal=c.name == state.name,)
                for c in self.topology.topology
            ]

            state.status = {}
        elif state.phase == "running":
            for client in self.topology.topology:
                local = search_client(client.name, state)
                if local != None:
                    local.address = client.address
                    local.replicatedBy = client.replicatedBy
                    local.replicating = client.replicating
                else:
                    state.computers.append(
                        computer_data_from(client, isLocal=client.name == state.name,)
                    )

        state.topologyUpdate.put_nowait(None)


@dataclass
class Reject:
    id: str

    def run(self, state: State, request: Request, conf):
        request.respond(Response(Reject(self.id)))


@dataclass
class Accept:
    id: str
    timeout: float = 0.500

    def run(self, state: State, request: Request, conf):
        request.respond(ResponseWithAck(self.id, Accept(self.id), self.timeout))


@dataclass
class CommitTo:
    target: str
    transaction: Union[DeleteTransaction, DeliverTransaction, ReceiveTransaction]

    def run(self, state: State, request: Request, conf):
        if self.transaction.apply(state):
            state.transactions[self.transaction.id] = self.transaction
            writeToFile(conf.stateFile, state)
            return Accept(self.transaction.id)
        else:
            return Reject(self.transaction.id)


@dataclass
class Commited:
    id: str

    def run(self, state: State, req, conf):
        del state.transactions[self.id]
        writeToFile(conf.stateFile, state)

        return NotifyFileSystemUpdate(state.name)


@dataclass
class Rollback:
    id: str

    def run(self, state: State, req, conf):
        state.transactions[self.id].rollback(state)
        del state.transactions[self.id]
        writeToFile(conf.stateFile, state)


@dataclass
class Suspend:
    requestTypes: List[Any]

    def run(self, state: State, req, conf):
        state.ignoreRequestsOfType.extend(self.requestTypes)


@dataclass
class Enable:
    requestTypes: List[Any]

    def run(self, state: State, req, conf):
        for r in self.requestTypes:
            if r in state.ignoreRequestsOfType:
                state.ignoreRequestsOfType.remove(r)


@dataclass
class NotifyFileSystemUpdate:
    name: str

    def run(self, state: State, req, conf):
        computer = state.find_computer(self.name)
        state.fileSystemUpdate.on_next((self.name, computer.fileSystem))


@dataclass
class RunTransaction:
    transaction: TransactionRequest

    def run(self, state: State, req, conf):
        state.transactionQueue.put_nowait(self.transaction)


@dataclass
class StreamFileSystemFrom:
    name: str

    def run(self, state: State, request: Request, conf):
        request.respond(
            Stream(
                asGenerator(
                    state.fileSystemUpdate.pipe(ops.filter(lambda x: x[0] == self.name))
                )
            )
        )


@dataclass
class UpdateFS:
    name: str
    fileSystem: FileSystem

    def run(self, state: State, request, conf):
        state.find_computer(self.name).fileSystem = self.fileSystem


@dataclass
class AnswerRequestWith:
    obj: Any

    def run(self, state: State, request: Request, conf):
        request.respond(Response(self.obj))
