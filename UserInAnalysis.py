# Importing Libraries 
import serial 
import time 
import PySimpleGUI as sg
import os

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

			UI Description: This python script genrates a GUI that processes
			user input for our team's SCARA robotic arm. A user can enter 
			the following input:

			* GCODE movement commands (G00 or G01) manually. These get
			written to the GCODE.txt file for further processing by a 
			separate script.

			* GCODE commands M-code commands that alter the status of the
			program (e.g., G21 to set units to mm or M02 to terminate).
			These get written to the STATUS.txt file for further processing 
			by a separate script.

			* One of five shapes that the user wants the robotic arm to copy
			(written to the DRAWING.txt file for further processing 
			by a separate script).

			* M06 or untensil selection that will inform the arm to swap in
			the specified utencil (pen, pencil, or crayon). The selection
			is written to the UTENSIL.txt file for further processing 
			by a separate script.

'''

# This string represents the command to be sent to the Arduino
command = ''

''' ESTABLISH ARDUINO CONNECTION '''
# Setting up the Arduino serial connection!
arduino = serial.Serial(port='COM6', baudrate=115200, timeout=.1)

# Give the connection time to establish
time.sleep(5)

''' SET UP UI '''
# Setting the GUI theme
sg.theme('DarkAmber')

# Global variables that impact displayed images
subs_size = 3
im_dim = 80

# The layout defines what is displayed in the window
layout = [  
            # This frame is responsible for selection of "shape" for the arm to draw
            [sg.Frame(title='Shape Selection', layout=[ 
                [sg.Button('SQUARE', image_source="../square.PNG", image_subsample=subs_size, image_size=(im_dim, im_dim)), 
                 sg.Button('TRIANGLE', image_source="../triangle.PNG", image_subsample=subs_size, image_size=(im_dim, im_dim)), 
                 sg.Button('CIRCLE', image_source="../circle.PNG", image_subsample=subs_size, image_size=(im_dim, im_dim)), 
                 sg.Button('HEXAGON', image_source="../hexagon.PNG", image_subsample=subs_size, image_size=(im_dim, im_dim)), 
                 sg.Button('STAR', image_source="../star.PNG", image_subsample=subs_size, image_size=(im_dim, im_dim)),]
            ])],
            # This frame is responsible for utensil selection (equivalent internally to using the "M06" command)
            [sg.Frame(title='Utensil Selection', layout=[
                [sg.Button('PEN'), 
                 sg.Button('PENCIL'), 
                 sg.Button('CRAYON'),]
            ])],
            # This frame is responsible for the user input of GCODE commands
            [sg.Frame(title='Manual GCode', layout=[
                [sg.Text('ENTER GCODE COMMAND:'), sg.InputText(do_not_clear=False), sg.Button('OK')]
            ])],
            # This frame provides a reference of available commands
            [sg.Frame(title='GCode Command Guide', layout=[
                [sg.Text('''Rapid Positioning: G00 X{x_coord} Y{y_coord}
                            \nLinear Interpolation: G01 X{x_coord} Y{y_coord} F{speed}
                            \nAbsolute Positioning: G90
                            \nRelative Positioning: G91
                            \nSet Units to Inches: G20
                            \nSet Units to Millimeters: G21
                            \nEnd the Program: M02
                            \nChange Writing Utensil: M06 T{tool_#}
                            \nSave Current State: M70
                            \nRestore Saved State: M72''')]
            ])],
        ]

# Create the Window
window = sg.Window('SCARA Robotic Arm Interface', layout)

''' REMEMBER: INPUT HANDLING IS NOW HAPPENING IN THE ARDUINO SCRIPT '''
# Event Loop to process "events" and get the "values" of the inputs
while True:
	event, values = window.read()
	if (event == sg.WIN_CLOSED):
		command='M02'
		break
	# If an event occurred, rather than user text input...
	elif(not values[0]):
		# Check if a drawing was selected
		if(event=="SQUARE" or event=="TRIANGLE" or event=="CIRCLE"or event=="HEXAGON" or event=="STAR"):
			command = event
		elif(event=="PEN"):
			command = 'M06 T01'
		elif(event=="PENCIL"):
			command = 'M06 T02'
		elif(event=="CRAYON"):
			command = 'M06 T03'
	# If not an event, then retrieve user text input
	else:
		command = values[0]

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
window.close()

# Set up UI
	# In the "new input loop->send to duino"



# # # PART 1: TEST GCODE COMMAND # # #

# # Read the sample GCode command from a text file
# gcode_file = open('SCARA-Arm-Command-Analysis/TEST_GCODE.txt', 'r')
# command = gcode_file.readline()
# gcode_file.close()
# # Append new line for processing by Arduino code
# command+="\n"
# # Write command to the serial port
# print("Python value sent: ")
# print(command)
# arduino.write(bytes(command, 'utf-8'))
# time.sleep(1)  
# # Establish that Arduino received message
# msg = arduino.readline()
# print ("Message from arduino: ")
# print (msg)
# # After Arduino processes command, read status
# time.sleep(0.5)
# print(arduino.readline())

# time.sleep(2)

# # # # PART 2: UTENSIL SWITCH TEST # # # 

# # Read the sample utensil switch command from a text file
# status_file = open('SCARA-Arm-Command-Analysis/TEST_UTENSIL.txt', 'r')
# command = status_file.readline()
# status_file.close()
# command+="\n"
# # Write command to the serial port
# print("Python value sent: ")
# print(command)
# arduino.write(bytes(command, 'utf-8'))
# time.sleep(1)
# # Establish that Arduino received message
# msg = arduino.readline()
# print ("Message from arduino: ")
# print (msg)
# # After Arduino processes command, read status
# time.sleep(0.5)
# print(arduino.readline())

# time.sleep(2)

# # # # PART 3: SHAPE DRAWING # # # 

# # Read the sample shape draw command from a text file
# shape_file = open('SCARA-Arm-Command-Analysis/TEST_SHAPE.txt', 'r')
# command = shape_file.readline()
# shape_file.close()
# # Write command to the serial port
# print("Python value sent: ")
# print(command)
# arduino.write(bytes(command, 'utf-8'))
# time.sleep(1)
# # Establish that Arduino received message
# msg = arduino.readline()
# print ("Message from arduino: ")
# print (msg)
# # After Arduino processes command, read status
# time.sleep(8)
# print(arduino.readline())
