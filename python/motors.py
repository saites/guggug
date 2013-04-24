#!/usr/bin/env python

import posix_ipc
import sys
import mmap
import struct
import time
from pyfirmata import Arduino, util

FORWARD = 1
BACKWARD = 0
SHM_SIZE = 16
CM_PER_COUNT = 0.031
DEGREES_PER_CM = 0.47
TURN_CORRECTION = 1.015
ENCODER_FILE = "/encoder"


board = Arduino('/dev/ttyACM0')


class Motor:
    def __init__(self, reversed, motorPin, brakePin, controlPin):
        self.reversed = reversed
        self.direction = FORWARD
        self.motorPin = board.get_pin('d:' + str(motorPin) + ':o')
        self.brakePin = board.get_pin('d:' + str(brakePin) + ':o')
        self.controlPin = board.get_pin('d:' + str(controlPin) + ':p')

    def stop(self):
        self.brakePin.write(1.0)
        self.speed(0.0)

    def run(self):
        self.brakePin.write(0.0)

    def speed(self, speed):
        self.controlPin.write(speed)

    def forward(self):
        self.motorPin.write(1 ^ self.reversed)

    def backward(self):
        self.motorPin.write(0 ^ self.reversed)

class Encoders:
    def __init__(self):
        self.counts = (0, 0)
        self.dist = (0, 0);
        self.delta = (0, 0);
        self.shm = posix_ipc.SharedMemory(ENCODER_FILE)
        self.map_file = mmap.mmap(self.shm.fd, self.shm.size)
        self.getDelta()

    def poll(self):
        self.map_file.seek(0)
        self.counts = tuple(struct.unpack('qq', self.map_file.read(SHM_SIZE)))
        lastdist = self.dist
        self.dist = tuple([x * CM_PER_COUNT for x in self.counts])
        self.delta = tuple([x - y for x, y in zip(self.dist, lastdist)])

    def getDelta(self):
        self.poll()
        return self.delta

    def getData(self):
        self.poll()
        return self.counts

class Robot:
    def __init__(self):
        self.lMotor = Motor(0, 12, 9, 3)
        self.rMotor = Motor(1, 13, 8, 11)
        self.encoders = Encoders()

    def move(self, direction, distance):
        dist = (distance, distance)
        if direction == 1:
            self.lMotor.forward()
            self.rMotor.forward()
        else:
            self.lMotor.backward()
            self.rMotor.backward()
        self.lMotor.run()
        self.rMotor.run()

        ticks = 0
        self.lMotor.speed(1.0)
        self.rMotor.speed(1.0)
        while dist[0] >= 0 or dist[1] >= 0:
            time.sleep(0.004)
            ticks += 1
            delta = self.encoders.getDelta()
            if direction == 0:
                delta = (delta[0] * TURN_CORRECTION, delta[1])

            dist = tuple([l - r for l, r in zip(dist, delta)])
            if dist[0] >= 100 or dist[1] >= 100:
                if dist[0] >= dist[1]:
                    self.lMotor.speed(0.1)
                    self.rMotor.speed(0.8)
                else:
                    self.lMotor.speed(0.8)
                    self.rMotor.speed(0.1)
            else:
                if dist[0] >= dist[1]:
                    self.lMotor.speed(0.0)
                    self.rMotor.speed(0.6)
                else:
                    self.lMotor.speed(0.6)
                    self.rMotor.speed(0.0)
        self.lMotor.stop()
        self.rMotor.stop()
        print self.encoders.getData()

    def turn(self, direction, degrees):
        degrees *= DEGREES_PER_CM
        dist = (degrees, degrees)
        if direction == 1:
            self.lMotor.forward()
            self.rMotor.backward()
        else:
            self.lMotor.backward()
            self.rMotor.forward()

        self.lMotor.run()
        self.rMotor.run()

        ticks = 0
        self.lMotor.speed(1.0)
        self.rMotor.speed(1.0)
        while dist[0] >= 0 or dist[1] >= 0:
            time.sleep(0.002)
            ticks += 1
            dist = tuple([l - r for l, r in zip(dist, self.encoders.getDelta())])
        self.lMotor.stop()
        self.rMotor.stop()
        print self.encoders.getData()

def main_function():
    gugug = Robot()

    for line in sys.stdin:
        tokens = line.split()
        if tokens[0] == 'move':
            gugug.move(tokens[1], tokens[2])
        elif tokens[0] == 'turn':
            gugug.turn(tokens[1], tokens[2])


if __name__ = '__main__':
    main_function()