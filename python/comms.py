#!/usr/bin/env python

import signal
import fcntl
import subprocess
import shlex
import select
import sys
import os
import posix_ipc
import mmap
import struct
import time

shm_size = 16
cm_per_count = 0.030875
interval = 0.5

def nonBlockRead(output):
    fd = output.fileno()
    fl = fcntl.fcntl(fd, fcntl.F_GETFL)
    fcntl.fcntl(fd, fcntl.F_SETFL, fl | os.O_NONBLOCK)
    try:
        return output.read()
    except:
        return ''

def read_encoder(map_file):
    """Reads in the current encoder value"""
    map_file.seek(0)
    return tuple(cm_per_count * x for x in  struct.unpack('qq', map_file.read(shm_size)))


def main_function():

    # setup up select polling object
    poller = select.epoll()
    proc_outs = {} # a hash table of file descriptors to processes

    # start the encoder server
    enc_cmd = "../c/encoder"
    enc_process = subprocess.Popen(enc_cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    proc_outs[enc_process.stdout.fileno()] = enc_process
    poller.register(enc_process.stdout, select.EPOLLIN)
    time.sleep(1) # need to wait a sec for encoder server to open file shm

    # open shared memory and map it to mapfile
    shm = posix_ipc.SharedMemory("/encoder")
    map_file = mmap.mmap(shm.fd, shm.size)

    # signal handling closure
    def signal_callback_handler(signum, frame):
        """ cleanup and exit """
        map_file.close()
        shm.close_fd()

        enc_process.send_signal(signal.SIGINT)
        # shm.unlink()
        sys.exit(0)

    # register our signal handler
    signal.signal(signal.SIGINT, signal_callback_handler)

    while True:
        for fd, flags in poller.poll(timeout=1):
            proc = proc_outs[fd]
            print proc.stdout.readline(),



if __name__ == '__main__':
    main_function()
