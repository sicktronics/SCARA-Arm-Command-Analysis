# Importing Libraries 
import serial 
import time 

'''
Parker Mayer
Junior Design II
Block 2 Demo
Description: This program establishes a serial connection with
			 the Arduino board and then sends three types of
			 user input: a sample GCODE command, a sample
			 writing utensil selection, and a sample shape
			 drawing request.

			 After the Arduino processes the commands, it sends
			 back the state of either the arm motors or the servo.
'''


# Setting up the Arduino serial connection!
arduino = serial.Serial(port='COM6', baudrate=115200, timeout=.1)

# Give the connection time to establish
time.sleep(5)

# # # PART 1: TEST GCODE COMMAND # # #

# Read the sample GCode command from a text file
gcode_file = open('SCARA-Arm-Command-Analysis/TEST_GCODE.txt', 'r')
command = gcode_file.readline()
gcode_file.close()
# Append new line for processing by Arduino code
command+="\n"
# Write command to the serial port
print("Python value sent: ")
print(command)
arduino.write(bytes(command, 'utf-8'))
time.sleep(1)  
# Establish that Arduino received message
msg = arduino.readline()
print ("Message from arduino: ")
print (msg)
# After Arduino processes command, read status
time.sleep(0.5)
print(arduino.readline())

time.sleep(2)

# # # PART 2: UTENSIL SWITCH TEST # # # 

# Read the sample utensil switch command from a text file
status_file = open('SCARA-Arm-Command-Analysis/TEST_UTENSIL.txt', 'r')
command = status_file.readline()
status_file.close()
command+="\n"
# Write command to the serial port
print("Python value sent: ")
print(command)
arduino.write(bytes(command, 'utf-8'))
time.sleep(1)
# Establish that Arduino received message
msg = arduino.readline()
print ("Message from arduino: ")
print (msg)
# After Arduino processes command, read status
time.sleep(0.5)
print(arduino.readline())

time.sleep(2)

# # # PART 3: SHAPE DRAWING # # # 

# Read the sample shape draw command from a text file
shape_file = open('SCARA-Arm-Command-Analysis/TEST_SHAPE.txt', 'r')
command = shape_file.readline()
shape_file.close()
# Write command to the serial port
print("Python value sent: ")
print(command)
arduino.write(bytes(command, 'utf-8'))
time.sleep(1)
# Establish that Arduino received message
msg = arduino.readline()
print ("Message from arduino: ")
print (msg)
# After Arduino processes command, read status
time.sleep(8)
print(arduino.readline())
