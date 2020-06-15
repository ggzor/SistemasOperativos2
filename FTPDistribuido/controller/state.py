from __future__ import annotations

import asyncio
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Dict, List, Optional, Set, Union

import uuid
from lenses import lens
from rx.core.typing import Subject
from rx.subject import BehaviorSubject

from utils import pickle_get_except, pickle_set_except

phase = lens["phase"]


@dataclass
class FileMetadata:
    # El nombre de la computadora origen
    src: str
    # La ruta absoluta con respecto a donde se ejecuto el programa
    path: Path
    # La clave para descargar
    key: str


@dataclass
class File:
    # Ruta relativa al programa en ejecución
    path: Path
    # Uno de ready, pending
    state: str
    # Las claves de los envíos pendientes
    uploads: set
    # Las claves de los borrados
    deletes: set
    # Tamaño en bytes
    size: int
    metadata: Optional[FileMetadata] = None


@dataclass
class FileSystem:
    # Path -> File
    # Path: Ruta relativa al directorio compartido
    files: Dict[Path, File]


# Transacciones
@dataclass
class FileData:
    name: str
    source: str
    path: Path
    relativePath: Path
    size: int
    # Clave con la que se puede solicitar
    key: str


@dataclass
class DeliverTransaction:
    id: str
    target: str
    destination: str
    # Pares (clave, rutaArchivo)
    # Clave para entregar
    deliver: List[(str, str)]

    def apply(self, state: State):
        for key, fileName in self.deliver:
            realFile = state.find_computer(self.target).find_file(fileName)

            if realFile == None:
                return False

            realFile.uploads.add(key)

        return True

    def rollback(self, state: State):
        for key, fileName in self.deliver:
            realFile = state.find_computer(self.target).find_file(fileName)

            if realFile == None:
                continue

            if key in realFile.uploads:
                realFile.uploads.remove(key)


@dataclass
class ReceiveTransaction:
    id: str
    target: str
    receive: List[FileData]

    def apply(self, state: State):
        computer = state.find_computer(self.target)
        for f in self.receive:
            current = computer.fileSystem.files.get(f.relativePath, None)

            # No está registrado
            if current == None:
                computer.fileSystem.files[f.relativePath] = File(
                    Path(computer.folder) / f.relativePath,
                    "pending",
                    set(),
                    set(),
                    f.size,
                    FileMetadata(f.source, f.path, f.key),
                )
            else:
                print("REGISTERED ALREADY :(")
                return False

        return True

    def rollback(self, state: State):
        computer = state.find_computer(self.target)

        for f in self.receive:
            current = computer.find_file(str(f.path))
            condition = (
                current != None and current.metadata and current.metadata.key == f.key
            )
            if condition:
                del computer.fileSystem.files[f.relativePath]


@dataclass
class DeleteTransaction:
    id: str
    target: str
    items: List[(str, str)]

    def apply(self, state: State):
        for key, fileName in self.items:
            realFile = state.find_computer(self.target).find_file(fileName)

            if realFile == None:
                return False

            realFile.deletes.add(key)

        return True

    def rollback(self, state: State):
        for key, fileName in self.items:
            realFile = state.find_computer(self.target).find_file(fileName)

            if realFile == None:
                continue

            if key in realFile.deletes:
                realFile.deletes.remove(key)


@dataclass
class AggregateTransaction:
    id: str
    target: str
    transactions: List[Union[ReceiveTransaction, DeleteTransaction, DeliverTransaction]]

    def apply(self, state: State):
        failed = False
        applied = []

        for tx in self.transactions:
            result = tx.apply(state)
            if result:
                applied.append(tx)
            else:
                tx.rollback(state)
                failed = True
                break

        if failed:
            for tx in applied:
                tx.rollback(state)

            return False

        return True

    def rollback(self, state: State):
        for tx in self.transactions:
            tx.rollback(state)


@dataclass
class ComputerData:
    name: str  # Relevante
    position: int
    address: str
    requestsPort: int
    folder: str
    # Quien lo replica
    replicatedBy: Optional[str]  # Relevante
    replicating: List[str]  # Relevante
    isLocal: bool
    fileSystem: Optional[FileSystem] = None  # Relevante
    sentFiles: Optional[int] = None
    receivedFiles: Optional[int] = None

    def find_file(self, name: str) -> File:
        if self.fileSystem != None:
            candidates = [
                f for f in self.fileSystem.files.values() if str(f.path) == name
            ]

            if len(candidates) > 0:
                return candidates[0]

        return None


@dataclass
class State:
    phase: str  # Relevante
    transactions: Dict[
        str, Union[DeliverTransaction, DeleteTransaction, ReceiveTransaction]
    ] = field(default_factory=dict)
    ignoreRequestsOfType: List[Any] = field(default_factory=list)
    name: Optional[str] = None  # Relevante
    computers: Optional[List[ComputerData]] = None  # Relevante
    status: Optional[Dict[str, str]] = None
    transactionQueue: asyncio.Queue = None
    # Ignorar
    fileSystemUpdate: Subject[(str, FileSystem)] = field(
        default_factory=lambda: BehaviorSubject(None)
    )
    # Ignorar
    topologyUpdate: asyncio.Queue = field(default_factory=asyncio.Queue)

    @property
    def accessible_computers(self):
        return [
            self.name,
            *self.find_computer(self.name).replicating,
        ]

    @property
    def current_computer(self):
        return self.find_computer(self.name)

    def find_computer(self, name: str) -> Optional[ComputerData]:
        candidates = [c for c in self.computers if c.name == name]

        if candidates:
            return candidates[0]

    transient_fields = {
        "status": lambda: {},
        "ignoreRequestsOfType": lambda: [],
        "transactionQueue": lambda: asyncio.Queue(),
        "fileSystemUpdate": lambda: BehaviorSubject(None),
        "topologyUpdate": asyncio.Queue,
    }

    # Específico de pickle
    def __getstate__(self):
        return pickle_get_except(self, State.transient_fields)

    def __setstate__(self, state):
        pickle_set_except(self, state, State.transient_fields)


@dataclass
class TransactionRequest:
    id: str
    requests: List[Union[DeliverTransaction, DeleteTransaction, ReceiveTransaction]]


@dataclass
class TransactionCommitted:
    id: str


@dataclass
class TransactionRejected:
    id: str
