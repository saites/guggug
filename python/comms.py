#!/usr/bin/env python

import signal
import sys
import posix_ipc
import mmap
import struct
import time

def read_encoder(mapfile):
    """Reads in the current encoder value"""
    mapfile.seek(0)
    return struct.unpack('q', mapfile.read(8))[0]


def main_function():
    # open shared memory and map it to mapfile
    shm = posix_ipc.SharedMemory("/encoder")
    mapfile = mmap.mmap(shm.fd, shm.size)

    # signal handling closure
    def signal_callback_handler():
        """ cleanup and exit """
        mapfile.close()
        shm.close_fd()
        shm.unlink()
        sys.exit(0)

    # register our signal handler
    signal.signal(signal.SIGINT, signal_callback_handler)

    while True:
        time.sleep(.3)
        print "Encoder value: " + str(read_encoder(mapfile))


if __name__ == '__main__':
    main_function()
