from sys import argv
import struct

struct_format = "ii10sxx"

with open(int(argv[1]), 'rb', buffering=0) as cToPython,\
     open(int(argv[2]), 'wb', buffering=0) as pythonToC:
    
    size = int.from_bytes(cToPython.read(4), "little")
    data = cToPython.read(size)
    
    x, y, t = struct.unpack(struct_format, data)
    t = t.split(b'\0')[0].decode('ascii')

    x //= 10
    y //= 10
    t = t[::-1]

    t = (t + "\0" * 10)[:10].encode('ascii')
    
    data = struct.pack(struct_format, x, y, t)
    pythonToC.write(data)
