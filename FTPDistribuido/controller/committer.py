from .state import (
    AggregateTransaction,
    State,
    TransactionCommitted,
    TransactionRejected,
)
from .actions import Reject, Accept

from asyncServer import streamQueue, Ack
from asyncClient import connectTo
from utils import readObject, writeObject

import asyncio
from dataclasses import dataclass
import logging
import traceback
from typing import List, Union


def get_computer_connection_params(name: str, state: State):
    c = state.find_computer(name)
    return (c.address, c.requestsPort)


async def applyTransaction(reader, writer, transaction):
    try:
        await writeObject(writer, transaction)
        response = await readObject(reader)
        return response
    except:
        return Reject(transaction.id)


async def ackToConnection(reader, writer):
    await writeObject(writer, Ack())


async def attemptConnection(transaction, state: State):
    addr, port = get_computer_connection_params(transaction.target, state)

    try:
        reader, writer = await asyncio.open_connection(addr, port)
        return (reader, writer, [transaction])
    except:
        return None


def get_replicator(name, state: State):
    return state.find_computer(name).replicatedBy


async def close_connections(connections):
    async def close_connection(conn):
        conn.close()
        await conn.wait_closed()

    await asyncio.gather(*map(close_connection, connections), return_exceptions=True)


async def routeConnections(transactions, state):
    results = await asyncio.gather(
        *map(lambda t: attemptConnection(t, state), transactions)
    )
    directory = {t.target: res for t, res in zip(transactions, results)}

    connected = True

    for t in transactions:
        if directory[t.target] == None:
            replica = get_replicator(t.target, state)
            if replica in directory:
                if directory[replica] != None:
                    directory[replica][2].append(t)
                else:
                    connected = False
                    break
            else:
                replicaParams = get_computer_connection_params(replica, state)
                conn = await connectTo(*replicaParams)

                if conn == None:
                    connected = False
                    break

                directory[replica] = (*conn, [t])

    if not connected:
        await close_connections(map(lambda x: x[1], filter(None, directory.values())))
        return None

    final_results = []
    for k, v in directory.items():
        if v != None:
            if len(v[2]) == 1:
                final_results.append((v[0], v[1], v[2][0]))
            else:
                final_results.append(
                    (v[0], v[1], AggregateTransaction(v[2][0].id, k, v[2]))
                )
    return final_results


async def commiter(queue, state: State, timeout: float = 0.500):
    async for commitRequest in streamQueue(queue):
        succeeded = False

        actions = None
        try:
            actions = await routeConnections(commitRequest.requests, state)

            if actions == None:
                yield TransactionRejected(commitRequest.id)
                # De lo contrario la ejecución va a continuar
                continue

            responses = await asyncio.wait_for(
                asyncio.gather(*map(lambda params: applyTransaction(*params), actions)),
                timeout,
            )

            if all([isinstance(res, Accept) for res in responses]):
                succeeded = True
                await asyncio.gather(
                    *map(lambda conn: ackToConnection(conn[0], conn[1]), actions)
                )
        except Exception as ex:
            tb = "\n".join(traceback.format_tb(ex.__traceback__))
            logging.getLogger("committer").warning(f"Unexpected: {ex}\n{tb}")
        finally:
            if actions != None:
                await close_connections(map(lambda x: x[1], actions))

            if succeeded:
                yield TransactionCommitted(commitRequest.id)
            else:
                yield TransactionRejected(commitRequest.id)
