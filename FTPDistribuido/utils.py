import struct
import pickle
from pathlib import Path
from typing import TypeVar

T = TypeVar("T")


def loadFromFile(f: str, default: T) -> T:
    if not Path(f).exists():
        return default

    with open(f, "rb") as f:
        return pickle.load(f)


def writeToFile(f: str, value):
    with open(f, "wb") as f:
        pickle.dump(value, f)


async def readObject(reader):
    size = struct.unpack("@I", await reader.read(4))[0]
    return pickle.loads(await reader.read(size))


def readObjectSync(reader):
    pass


async def writeObject(writer, obj):
    obj = pickle.dumps(obj)
    writer.write(struct.pack("@I", len(obj)))
    await writer.drain()

    writer.write(obj)
    await writer.drain()


def pickle_get_except(obj, transient):
    data = {**vars(obj)}

    for k in transient:
        data.pop(k)

    return data


def pickle_set_except(obj, state, transient):
    for k, v in state.items():
        setattr(obj, k, v)

    for k, v in transient.items():
        setattr(obj, k, v())
