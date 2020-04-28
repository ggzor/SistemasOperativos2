import asyncio
import pickle
import struct


async def _recv_header(conn, buff_sz):
    """Read the length of the packet stream
    Return.
      (length of packet, extra_bytes)"""
    recv_buffer = b""
    bytes_len = struct.Struct("I").size

    try:
        recv_buffer = await conn.readexactly(bytes_len)
    except asyncio.IncompleteReadError:
        return None

    return struct.unpack("@I", recv_buffer)[0]


async def recv_packet(conn, buff_sz=4096):
    """Receive one packet from the connection"""
    size_packet = await _recv_header(conn, buff_sz)

    if not size_packet:
        return None

    try:
        buffer_packet = await conn.readexactly(size_packet)
    except asyncio.IncompleteReadError:
        return None

    return pickle.loads(buffer_packet)


async def send_packet(conn, packet):
    raw_packet = pickle.dumps(packet)
    len_packet = struct.pack("@I", len(raw_packet))
    conn.write(len_packet + raw_packet)
    await conn.drain()
