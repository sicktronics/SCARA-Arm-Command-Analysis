# Importing Libraries 
import serial 
import time 
# Setting up the duino!
arduino = serial.Serial(port='COM7', baudrate=115200, timeout=.1)

# boot up time
time.sleep(5)

# # # PART 1: TEST GCODE COMMAND # # #

# Write the drawing selection to a file
gcode_file = open('SCARA-Arm-Command-Analysis/TEST_GCODE.txt', 'r')
command = gcode_file.readline()
gcode_file.close()
command+="\n"
# Serial write section
print("Python value sent: ")
print(command)
arduino.write(bytes(command, 'utf-8'))
time.sleep(1) # with the port open, the response will be buffered 
msg = arduino.readline() # read everything in the input buffer
print ("Message from arduino: ")
print (msg)
time.sleep(0.5)
print(arduino.readline())

# # # PART 2: UTENSIL SWITCH TEST # # # 
	
	