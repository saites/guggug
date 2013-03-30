import time
from robot import *

robot = Robot()

x=""
while(1):
	x = raw_input('Enter a command: ')
	print x
	try:
		eval(x)
	except:
		print "invalid command"
