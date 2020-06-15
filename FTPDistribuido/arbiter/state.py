from __future__ import annotations

import asyncio
from dataclasses import dataclass
from typing import List, Optional
from utils import pickle_get_except, pickle_set_except

from lenses import lens


@dataclass
class Client:
    position: int
    name: str
    address: str
    requestsPort: int
    folder: str
    state: str
    replicating: List[str]
    replicatedBy: Optional[str]
    messages: asyncio.Queue
    healthCheckPort: int

    def follow(self, other: Client):
        assert other.replicatedBy == None

        self.replicating.append(other.name)
        other.replicatedBy = self.name

    def unfollow(self, other: Client):
        self.replicating.remove(other.name)
        other.replicatedBy = None

    transient_fields = {"messages": lambda: asyncio.Queue(), "state": lambda: "pending"}

    # Específico de pickle
    def __getstate__(self):
        return pickle_get_except(self, Client.transient_fields)

    def __setstate__(self, state):
        return pickle_set_except(self, state, Client.transient_fields)


@dataclass
class TransientState:
    topologyUpdate: asyncio.Queue


@dataclass
class State:
    counter: int
    clients: List[Client]
    transient: TransientState

    def find_client_where(self, pred) -> Client:
        candidates = [c for c in self.clients if pred(c)]

        if candidates:
            return candidates[0]

    def find_client(self, name):
        return self.find_client_where(lambda c: c.name == name)

    def broadcast(self, message):
        for c in self.clients:
            c.messages.put_nowait(message)

    transient_fields = {"transient": lambda: TransientState(asyncio.Queue())}

    # Específico de pickle
    def __getstate__(self):
        return pickle_get_except(self, State.transient_fields)

    def __setstate__(self, state):
        pickle_set_except(self, state, State.transient_fields)


allClients = lens.clients.Each()
allNames = allClients.name.collect()
allAddresses = allClients.address.collect()
