#include <AFMotor.h>
#include <Servo.h>
#include <math.h>
// #include <fmt.h>

// Setting up two stepper motors and a servo

// for num steps: 360/degrees per step
AF_Stepper m1(200,1);
AF_Stepper m2(200,2);
int m1_steps = 0;
int m2_steps = 0;
float m1_angle = 0.0;
float m2_angle = 0.0;

Servo myservo;
// Need to use mm
float len1 = 100.0;
float len2 = 100.0;

/*
* Parker Mayer
* Junior Design II
* Block 2 Demo
* Description: This program receives a command via a serial connection from a Python script.
               (Either a new GCODE command, a utensil switch command, or a shape draw command.)
               The program first interprets the incoming command to see whether it's GCode, an MCode,
               or a shape.
               - If it's a GCode movement command, it uses a line algorithm to move the stepper motors appropriately.
               - If it's a utensil-switch MCode command, it moves the servo corresponding to the utensil switch mechanism.
               - If it's a shape to draw, the corresponding macro executes.
* NOTE: For GCODE interpretation, I used this article as my main reference:
*   https://www.marginallyclever.com/2013/08/how-to-build-an-2-axis-arduino-cnc-gcode-interpreter/
*/

// Global variables
#define BAUD (115200) // Speed of Arduino comms
#define MAX_BUFF (64) // Max message length sent to Arduino

char buffer[MAX_BUFF]; // where we store the message until we get a ;
int sofar;             // How much is in the buffer 

#define VERSION (1.0) // Program version
#define MIN_FEEDRATE (50.0) // Min, max feedrate (need to update, this is just for testing)
#define MAX_FEEDRATE (100.0)
float fr = 100.0;           // Current feedrate
float nfr = 50.0;           // New feedrate
int mode_abs=1;             // Whether the current mode is absolute or relative (absolute by default)
int mode_mm=1;              // Whether the units are mm or in (mm by default)
float px = 0.0;             // x, y position
float py = 0.0;
long step_delay=1000000.0/fr; // Delay between step commands
int current = 1;            // Current utensil--by default, 1 - pen


// Start up code to run only once
void setup(){

  myservo.attach(9); // Set up the servo pin
  myservo.write(84); // This is the uncalibrated stop PWM for the servo
  m1.setSpeed(10);   // Motor set up, determine initial speed
  m2.setSpeed(10);
  m1.release();
  m2.release();

  Serial.begin(BAUD);   // Open the serial connection
  Serial.setTimeout(1); // Set timeout

  setFeedrate(100); // set default speed
  signalReady();    // Clear the buffer for new input
}

// Code that loops repeatedly
void loop(){

  // Listen for commands
  while(!Serial.available()) {} // Wait for data to arrive
  if(Serial.available()){       // If something is available

    // Read the incoming string and send it back
    // to the Python script
    String test = Serial.readString();
    Serial.print(test);

    // Check for shapes!
    if(test.equals("SQUARE\n")){
      drawSquare(); // Execute macro
    }
    if(test.equals("TRIANGLE\n")){
      drawTriangle(); // Execute macro
    } // TO DO: add more shapes....
    if(test.equals("HEXAGON\n")){
      drawHexagon(); // Execute macro
    } // TO DO: add more shapes....

    // Add each character of the string to the buffer
    for(int j = 0; j < test.length(); j++){
      char c = test[j];
      // IF there's room in the buffer, store the byte.
      // this prevents loss of data if the buffer is full.
      if(sofar < MAX_BUFF) buffer[sofar++]=c;
      // IF we got a return character (\n) then the message is done
      if(c=='\n'){
        // Serial.print(F("\r\n")); // for debugging
        // strings gotta end with a \0
        buffer[sofar]=0;
        processCommand(); // DO SOMETHING with the command
        signalReady();    // Clear the buffer for a new command
      }
    }
  }
}

/**
 * Look for character /code/ in the buffer and read the float that immediately follows it.
 * @return the value found.  If nothing is found, /val/ is returned.
 * @input code the character to look for.
 * @input val the return value if /code/ is not found.
 **/
float parsenumber(char code,float val) {
  char *ptr=buffer;  // Start at the beginning of buffer
  while((long)ptr > 1 && (*ptr) && (long)ptr < (long)buffer+sofar) {  // walk to the end
    if(*ptr==code) {  // If you find code on your walk,
      return atof(ptr+1);  // Convert the digits that follow into a float and return it
    }
    ptr=strchr(ptr,' ')+1;  // Take a step from here to the letter after the next space
  }
  return val;  // End reached, nothing found, return default val.
}

/*
  Prepares input buffer to receive new message;
  tells serial connected device it is ready for more
*/
void signalReady(){
  sofar = 0; // Clear the input buffer
  // Serial.print(F("> ")); // Signal that we're ready to receive input
}


// INTERPRETING COMMANDS

/* Read input buffer and find any recognized commands. One G or M command per line. */
void processCommand(){
  // Look for commands that start with a "G"
  int cmd=parsenumber('G',-1);
  switch(cmd) {
    // G00
    case 0: // move in a line
    // G01
    case 1: // move in a line
      setFeedrate(parsenumber('F', fr));
      // calling line and passing whether mode is absolute or not
      line( parsenumber('X', (mode_abs?px:0) + (mode_abs?0:px)), parsenumber('Y', (mode_abs?py:0) +  (mode_abs?0:py)) );
      // Afterwards, print current x, y positions
      Serial.print("Current x position: ");
      Serial.print(px);
      Serial.print(" Current y position: ");
      Serial.print(py);
      break;

    // G90 - absolute mode
    case 90: mode_abs=1; break;
    // G91 - relative mode - TO DO: account for relative mode in the program
    case 91: mode_abs=1; break;

    // TO DO: fill in G20
    case 20: break; // TO DO: account for inches
    // G21 - unit is mm
    case 21: mode_mm=1; break;

    default: break;
  }

  // Look for commands that start with an "M"
  cmd=parsenumber('M',-1);
  switch(cmd){
    // M02 - shut down
    case 2:
      servoMove(1);
      line(05.0, 05.0);
    // M06 - switch utensil
    case 6: // Switch utensil
      // Determine whether pen, pencil, or crayon via T-code
      int whichUtensil = parsenumber('T',-1);
      servoMove(whichUtensil);
      break;
    // M18 - release motors
    case 18:
      // release motor 1
      m1.release();
      // relesase motor 2
      m2.release();
      Serial.print("Release the motors!");
      break;
    // M72 - Restore system defaults
    case 72:
      mode_abs=1;
      mode_mm=1;
      Serial.print("Restore system default seetings! (Mode: absolute. Units: mm)");
      break;

    default: 
      break;
  }
  // If string has no G or M commands, it will reach this place
  // and the Arduino will ignore it....
}

// Macro for drawing a square...feeds coordinates to line function.

// IN MM
void drawSquare(){
  line(0.0, 0.0);
  line(50.0, 50.0);
  line(50.0, 100.0);
  line(100.0, 100.0);
  line(100.0, 50.0);
  line(0.0, 0.0);

  // Theoretically, positions should be the same after completing the square
  Serial.print("Current x position: ");
  Serial.print(px);
  Serial.print(" Current y position: ");
  Serial.print(py);
}
void drawTriangle(){
  line(0.0, 0.0);
  line(50.0, 50.0);
  line(75.0, 100.0);
  line(100.0, 50.0);
  line(50.0, 50.0);
  line(0.0, 0.0);
  // Theoretically, positions should be the same after completing the square
  Serial.print("Current x position: ");
  Serial.print(px);
  Serial.print(" Current y position: ");
  Serial.print(py);
}
// See parker's notes for math
void drawHexagon(){
  line(0.0, 0.0);
  line(50.0, 50.0);
  line(50.0, 86.6);
  line(0.0, 173.2);
  line(50.0, 259.8);
  line(150.0, 259.8);
  line(200.0, 173.2);
  line(150.0, 86.6);
  line(50.0, 86.6);
  line(0.0, 0.0);

  // Theoretically, positions should be the same after completing the square
  Serial.print("Current x position: ");
  Serial.print(px);
  Serial.print(" Current y position: ");
  Serial.print(py);
}

// MOVEMENT ALGORITHM

// Supporting functions


/*
  Using Bresenham's line algorithm to move both motors 
  @input newx the destination x position
  @input newy the destination y position

  NOTE: Need to port to Inverse Kinematics for SCARA arm
*/
void line(float newx, float newy){

  // CHANGE : Old to new VVV
  long dx=newx-px;      // x, y positional difference
  long dy=newy-py;
  int dirx=dx > 0?1:-1; // direction to move
  int diry=dy > 0?1:-1; 
  dx=abs(dx);           // abs value macro
  dy=abs(dy);
  long i;
  long over=0;
  // If x difference is greater than y
  if(dx > dy){
    for(i=0; i<dx; i++){
      // m1 takes steps in dirx
      m1.onestep(dirx, SINGLE);
      over+=dy;
      // If y difference is > x difference
      if(over>=dx){
        over-=dx;
        // m2 takes steps in diry
        m2.onestep(diry, SINGLE);
      }
      pause(step_delay); // step_delay is a global connected to feed rate
    }    
    // TO DO: test limits here
  } else {

    for(i=0; i<dy; ++i){
      // m2 takes one step in diry
      m2.onestep(diry, SINGLE);
      // If x difference is > y difference
      over+=dx;
      if(over>=dy){
        over-=dy;
        // m1 takes a step in dirx
        m1.onestep(dirx, SINGLE);
      }
      pause(step_delay);
    }
  }

  // Update logical position
  px+=dx*dirx;
  py+=dy*diry;
}

/* 
   Delay for # of microseconds
   @input how many milliseconds to wait
*/
void pause(long ms){
  delay(ms/1000);
  delayMicroseconds(ms%1000);
}

/* 
   Setting the feedrate (speed motors will move) 
   @input nfr the new speed in steps/second
*/
void setFeedrate(float nfr){
  if(fr==nfr) return; // same as last time? leave it!
  if(nfr > MAX_FEEDRATE || nfr < MIN_FEEDRATE) {
    Serial.print(F("New feed rate must be greater than "));
    Serial.print(MIN_FEEDRATE);
    Serial.print(F("steps/s and less than "));
    Serial.print(MAX_FEEDRATE);
    Serial.println(F("steps/s."));
    return;
  }
  step_delay=1000000.0/nfr;
  fr=nfr;
}

/* 
   Deciding how to make the servo move based on current utensil
   and requested new utensil.
   @input uten is the code for utensil (1, 2, or 3)
*/
void servoMove(int uten){

  switch(current){
    // If starting from utensil 1 - pen
    case 1:
        if(uten==1){
          myservo.write(84); // this is the uncalibrated stop

        } else if(uten==2){
          myservo.write(180); // 
          delay(500);
          myservo.write(84); // this is the uncalibrated stop

        } else if(uten==3){
          myservo.write(0); // 
          delay(500);
          myservo.write(84); // this is the uncalibrated stop
        }
        break;
    // If starting from utensil 2 - pencil
    case 2:
      if(uten==1){
        myservo.write(90); // 
        delay(500);
        myservo.write(84); // this is the uncalibrated stop

      } else if(uten==2){
        myservo.write(84); // this is the uncalibrated stop

      } else if(uten==3){
        myservo.write(0); //
        delay(500);
        myservo.write(84); // this is the uncalibrated stop
      }
      break;
    // If starting from utensil 3 - crayon
    case 3:
      if(uten==1){
        myservo.write(90); //
        delay(500);
        myservo.write(84); // this is the uncalibrated stop

      } else if(uten==2){
        myservo.write(180); //
        delay(500);
        myservo.write(84); // this is the uncalibrated stop

      } else if(uten==3){
        myservo.write(84); // this is the uncalibrated stop
      }
        default: break;
      }

  current = uten;
  // Send the current utensil to the Python script
  Serial.print("Current utensil: " );
  Serial.print(current);
}


