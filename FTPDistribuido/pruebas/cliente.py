import asyncio
import struct
import sys


async def client():
    reader, writer = await asyncio.open_connection("127.0.0.1", 1234)

    while True:
        input("Press enter to request an increment...")

        writer.write(b"\0")
        await writer.drain()

        response = int.from_bytes(await reader.read(1), sys.byteorder)
        if response:
            print("\033[32mAccepted\033[0m")
        else:
            print("\033[31mRejected\033[0m")


asyncio.run(client())
