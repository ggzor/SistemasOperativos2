from .state import State, File
from .requests import FileWatchUpdate

import asyncio
from pathlib import Path


async def watchFileSystem(state: State, config):
    directory = Path(config.folder)
    while True:
        if state.phase == "running":
            computer = state.find_computer(state.name)
            files = set(f for f in directory.rglob("*") if f.is_file())
            current_files = (
                {i.path for i in computer.fileSystem.files.values()}
                if computer.fileSystem
                else set()
            )
            missing_files = files - current_files

            if missing_files:
                yield FileWatchUpdate(
                    {
                        f.relative_to(config.folder): File(
                            f, "ready", set(), set(), f.stat().st_size
                        )
                        for f in missing_files
                    }
                )

        await asyncio.sleep(config.fileWatchInterval)
