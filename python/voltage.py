from pyfirmata import Arduino, util
import time

board = Arduino('/dev/ttyACM0')
voltPin = 0

voltagePin = board.get_pin('a:'+str(voltPin)+':i')
voltagePin.enable_reporting()

while(1):
	voltval = voltagePin.read()
	print "" + str(voltval)
	time.sleep(1)
	
