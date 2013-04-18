from pyfirmata import Arduino, util
import time

refVoltage = 4.60
R1 = 3050.0
R2 = 1036.0
vdivide = (R1+R2)/R2
vscale = refVoltage * vdivide

board = Arduino('/dev/ttyACM0')
analogPin = 5

it = util.Iterator(board)
it.start()
voltPin = board.get_pin('a:'+str(analogPin)+':i')
voltPin.enable_reporting()

avg10 = [0 for i in range(10)]

while(1):
	voltval = voltPin.read()*vscale
	avg10.insert(0, voltval)
	avg10.pop()
	avg = 0.0
	for n in avg10:
		avg += n
	print avg / len(avg10)
	time.sleep(.5)
	