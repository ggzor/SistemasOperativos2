from .state import Client, State
from utils import writeToFile
from asyncServer import Stream, streamQueue, Request

import asyncio
from dataclasses import dataclass
from typing import List, Optional

# Respuestas
@dataclass
class PositionAssigned:
    position: int


@dataclass
class Node:
    name: str
    position: int
    address: str
    requestsPort: int
    pullServerPort: int
    folder: str
    replicatedBy: Optional[str]
    replicating: List[str]


@dataclass
class Topology:
    topology: List[Node]


def calculate_topology(state: State) -> Topology:
    return Topology(
        [
            Node(
                c.name,
                c.position,
                c.address,
                c.requestsPort,
                c.pullServerPort,
                c.folder,
                c.replicatedBy,
                c.replicating,
            )
            for c in state.clients
        ]
    )


@dataclass
class NodeStatus:
    name: str
    state: str


@dataclass
class Statuses:
    statuses: List[NodeStatus]


def get_statuses(state: State) -> Statuses:
    return Statuses([NodeStatus(c.name, c.state) for c in state.clients])


@dataclass
class RegisterNewClient:
    name: str
    requestsPort: int
    pullServerPort: int
    folder: str
    healthCheckPort: int

    def run(self, state: State, req: Request, config):
        state.counter += 1

        client = Client(
            state.counter,
            self.name,
            req.address,
            self.requestsPort,
            self.pullServerPort,
            self.folder,
            "pending",
            [],
            None,
            asyncio.Queue(),
            self.healthCheckPort,
        )

        if len(state.clients) > 0:
            # Si hay un cliente sin par
            if len(state.clients) % 2 == 1:
                notPaired = state.find_client_where(lambda c: len(c.replicating) == 0)

                # Si hay un servidor replicándolo, removerlo
                if notPaired.replicatedBy is not None:
                    multiple = state.find_client(notPaired.replicatedBy)

                    multiple.unfollow(notPaired)

                # Emparejar clientes
                client.follow(notPaired)
                notPaired.follow(client)
            # Si todos tienen pareja
            else:
                # Buscar el primer cliente, prefiriendo los conectados
                follower: Client = sorted(
                    state.clients,
                    key=lambda c: ["online", "pending", "replica", "offline"].index(
                        c.state
                    ),
                )[0]

                follower.follow(client)

        state.clients.append(client)

        return NotifyTopologyChange()


@dataclass
class StreamClient:
    name: str

    def run(self, state: State, request: Request, config):
        client = [c for c in state.clients if c.name == self.name][0]

        client.messages = asyncio.Queue()
        client.messages.put_nowait(PositionAssigned(client.position))
        client.messages.put_nowait(calculate_topology(state))
        client.messages.put_nowait(get_statuses(state))

        request.respond(Stream(streamQueue(client.messages)))


@dataclass
class NotifyTopologyChange:
    def run(self, state: State, req, conf):
        # Sólo notificar la actualización
        state.transient.topologyUpdate.put_nowait(None)
        state.broadcast(calculate_topology(state))


@dataclass
class NotifyStatusChange:
    statuses: Statuses

    def run(self, state: State, req, conf):
        for client in self.statuses.statuses:
            state.find_client(client.name).state = client.state

        state.broadcast(self.statuses)
