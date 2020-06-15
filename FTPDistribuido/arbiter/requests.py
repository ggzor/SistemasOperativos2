from .state import allAddresses, allNames, State
from .actions import *

from asyncServer import Request

from dataclasses import dataclass
from functools import singledispatch

# Procesamiento de solicitudes
@singledispatch
def processRequest(request, state: State, fullRequest: Request):
    return [], []


@dataclass
class Register:
    name: str
    requestsPort: int
    folder: str
    healthCheck: int = 4321


@processRequest.register
def _(request: Register, state: State, fullRequest: Request):
    names = allNames(state)

    if request.name not in names:
        return (
            RegisterNewClient(
                request.name, request.requestsPort, request.folder, request.healthCheck
            ),
            StreamClient(request.name),
        )
    else:
        return [], StreamClient(request.name)
