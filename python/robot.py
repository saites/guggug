from pyfirmata import Arduino, util

board = Arduino('/dev/ttyACM0')
it = util.Iterator(board)
it.start()

PI = 3.14159
FORWARD = 1
BACKWARD = 0
FULLSPEED = 1.0
HALFSPEED = 0.5
STOPPED = 0.0
HIGH = 1.0
LOW = 0.0

class voltMeter:
	def __init__(self, pin, R1, R2, ref):
		self.pin = board.get_pin('a:'+str(pin)+':i')
		pin.enable_reporting()
		self.refVoltage = ref
		vscale = ref * (R1+R2)/R2

class LED:
    def __init__(self, pin):
        self.pin = board.get_pin('d:'+str(pin)+':p')

    def turnOn(self):
        self.pin.write(HIGH)

    def turnOff(self):
        self.pin.write(LOW)

class Motor:
	def __init__(self, motorPin, brakePin, controlPin):
		self.motorPin = board.get_pin('d:'+str(motorPin)+':o')
		self.brakePin = board.get_pin('d:'+str(brakePin)+':o')
		self.controlPin = board.get_pin('d:'+str(controlPin)+':p')
		
	def move(self, speed, direction):
		if(speed < 0):
			speed *= -1
			if(direction == FORWARD):
				direction = BACKWARD
			else:
				direction = FORWARD
		if(speed > 255):
			return
		elif(speed > 1):
			speed /= 255
		self.motorPin.write(direction)
		self.brakePin.write(LOW)
		self.controlPin.write(speed)
	
	def moveForward(self, speed):
		self.move(speed, FORWARD)
		
	def moveBackward(self, speed):
		self.move(speed, BACKWARD)
		
	def stop(self):
		self.brakePin.write(HIGH)
		self.controlPin.write(STOPPED)
		
class Robot:
	def __init__(self):
		self.motorA = Motor(12, 9, 3)
		self.motorB = Motor(13, 8, 11)
		self.greenLED = LED(4)
		self.redLED = LED(7)

	def turnOnGreen(self):
		self.greenLED.turnOn()
    
	def turnOffGreen(self):
		self.greenLED.turnOff()

	def turnOnRed(self):
		self.redLED.turnOn()
	
	def turnOnGreen(self):
		self.redLED.turnOff()

	def setGreen(self, val):
		self.greenLED.setTo(val)

	def move(self, speed, angle):
		"""PI/2 = FORWARD"""
		self.motorA.moveForward(speed * angle/PI)
		self.motorB.moveForward(speed * (-1 * angle + PI)/PI)
		
	def moveForward(self, speed):
		self.move(speed, PI/2)
	
	def moveBackward(self, speed):
		self.move(-1 * speed, PI/2)
		
	def stop(self):
		self.motorA.stop()
		self.motorB.stop()
	
	def turnLeft(self, speed):
		self.motorA.moveForward(speed)
		self.motorB.moveBackward(speed)
	
	def turnRight(self, speed):
		self.motorA.moveBackward(speed)
		self.motorB.moveForward(speed)
