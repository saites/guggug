#!/usr/bin/env python

import signal
import sys
import posix_ipc
import mmap
import struct
import time

shm_size = 16
cmpercount = 0.030875
interval = 0.5

def read_encoder(mapfile):
    """Reads in the current encoder value"""
    mapfile.seek(0)
    return tuple(cmpercount * x for x in  struct.unpack('qq', mapfile.read(shm_size)))


def main_function():
    # open shared memory and map it to mapfile
    shm = posix_ipc.SharedMemory("/encoder")
    mapfile = mmap.mmap(shm.fd, shm.size)

    # signal handling closure
    def signal_callback_handler(signum, frame):
        """ cleanup and exit """
        mapfile.close()
        shm.close_fd()
        # shm.unlink()
        sys.exit(0)

    # register our signal handler
    signal.signal(signal.SIGINT, signal_callback_handler)

    lastdist = read_encoder(mapfile)
    while True:
        time.sleep(interval)
        dist = read_encoder(mapfile)
        delta = tuple(a - b for a, b in zip(dist, lastdist))
        print "Dist:\t" + str(dist) + " cm"
        print "Delta:\t" + str(delta) + " cm"
        print "Speed:\t" +  str(tuple(a/interval for a in delta)) + " cm/s"
        lastdist = dist


if __name__ == '__main__':
    main_function()
