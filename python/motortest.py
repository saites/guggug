import time
from robot import *

robot = Robot()

print "move forward"
robot.moveForward(FULLSPEED)
time.sleep(3)
robot.stop()
time.sleep(2)

print "move backward"
robot.moveBackward(HALFSPEED)
time.sleep(3)
robot.stop()
time.sleep(2)

print "turn left"
robot.turnLeft(HALFSPEED)
time.sleep(3)
robot.stop()
time.sleep(2)

print "turn right"
robot.turnRight(HALFSPEED)
time.sleep(3)
robot.stop()
time.sleep(2)

print "move fullspeed, pi/4"
robot.move(FULLSPEED, PI / 4)
time.sleep(3)
robot.stop()
time.sleep(2)

print "move fullspeed, 3*pi/4"
robot.move(FULLSPEED, 3*PI / 4)
time.sleep(3)
robot.stop()
time.sleep(2)
