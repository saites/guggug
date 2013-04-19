#!/usr/bin/env python

import os
import posix_ipc
import mmap
import struct
import time

def read_encoder(mapfile):
    """Reads in the current encoder value"""
    mapfile.seek(0)
    return struct.unpack('q', mapfile.read(8))[0]

if __name__ == '__main__':
    shm = posix_ipc.SharedMemory("/encoder")
    mapfile = mmap.mmap(shm.fd, shm.size)

    while True:
        time.sleep(.3)
        print "Encoder value: " + str(read_encoder(mapfile))
