#!/usr/bin/env python

import signal
import fcntl
import subprocess
import shlex
import select
import sys
import os
import time

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
    enc_process = subprocess.Popen(enc_cmd, bufsize=1, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    proc_outs[enc_process.stdout.fileno()] = enc_process
    poller.register(enc_process.stdout, select.EPOLLIN)

    # start the motor server
    mot_cmd = "./motors.py"
    mot_process = subprocess.Popen(mot_cmd, bufsize=1, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    proc_outs[mot_process.stdout.fileno()] = mot_process
    poller.register(mot_process.stdout, select.EPOLLIN)

    # start speech processing
    aud_cmd = "../speech/src/recognize"
    aud_process = subprocess.Popen(aud_cmd, bufsize=1, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    proc_outs[aud_process.stdout.fileno()] = aud_process
    poller.register(aud_process.stdout, select.EPOLLIN)

    # start video processing
    vid_cmd = "../vision/turkeybaster"
    vid_process = subprocess.Popen(vid_cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    proc_outs[vid_process.stdout.fileno()] = vid_process
    poller.register(vid_process.stdout, select.EPOLLIN)

    # signal handling closure
    def signal_callback_handler(signum, frame):
        """ cleanup and exit """
        enc_process.send_signal(signal.SIGINT)
        vid_process.send_signal(signal.SIGKILL)
        aud_process.send_signal(signal.SIGKILL)
        mot_process.stdin.write("quit\n")
        sys.exit(0)

    # register our signal handler
    signal.signal(signal.SIGINT, signal_callback_handler)

    ignore_vid = True

    while True:
        for fd, flags in poller.poll(timeout=1):
            proc = proc_outs[fd]
            string = proc.stdout.readline()
            if len(string) == 0:
                continue
            tokens = string.split()
            if proc is vid_process and ignore_vid:
                continue;
            if string.strip() == "START TRACKING":
                print "TRACKING ON"
                mot_process.stdin.write("LEDON GREEN\n")
                ignore_vid = False
            elif string.strip() == "STOP TRACKING":
                print "TRACKING OFF"
                mot_process.stdin.write("LEDOFF GREEN\n")
                ignore_vid = True
            elif tokens[0] == "MOVE" or tokens[0] == "TURN" or tokens[0] == "LEDON" or tokens[0] == "LEDOFF":
                print "MOTOR COMMAND: " + string
                mot_process.stdin.write(string)
            else:
                print "UNRECOGNIZED: " + string




if __name__ == '__main__':
    main_function()
