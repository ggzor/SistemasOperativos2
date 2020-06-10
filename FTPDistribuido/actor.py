from aiostream.stream import merge

import logging
import pprint

actorLogger = logging.getLogger("Actor")
pp = pprint.PrettyPrinter(indent=2, width=88)


async def runActor(sources, initialState, processMessage, runAction, commit):
    state = initialState
    actorLogger.debug(f"Initial State:\n{pp.pformat(state)}")

    async with merge(*sources).stream() as stream:
        async for message in stream:
            actorLogger.debug(f"Message Received: \n{pp.pformat(message)}")
            actions = processMessage(message, state)

            actions, postActions = [
                acts if isinstance(acts, list) else [acts] for acts in actions
            ]

            if actions == postActions == []:
                actorLogger.warning(
                    f"No actions for message:\n{pp.pformat(message)}"
                    + f"\nState: {pp.pformat(state)}"
                )
                continue

            actorLogger.debug(f"Actions:\n{pp.pformat(actions)}")
            actorLogger.debug(f"Post-actions:\n{pp.pformat(postActions)}")

            runActions(runAction, actions, state, message)
            commit(state)
            runActions(runAction, postActions, state, message)

            actorLogger.debug(f"After commit:\n{pp.pformat(state)}")


def runActions(runAction, actions, state, message):
    while actions:
        action = actions.pop(0)
        newActions = runAction(action, state, message)

        if newActions != None:
            if not isinstance(newActions, list):
                newActions = [newActions]

            actorLogger.debug(f"New actions generated:\n{pp.pformat(newActions)}")

            actions.extend(newActions)
