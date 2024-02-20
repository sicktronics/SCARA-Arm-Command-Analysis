#include <AFMotor.h>
#include <Servo.h>

AF_Stepper m1(200,1);
AF_Stepper m2(200,2);

/*
* Parker Mayer
* https://www.marginallyclever.com/2013/08/how-to-build-an-2-axis-arduino-cnc-gcode-interpreter/
*/

// Variables
#define BAUD (115200) // Speed of Arduino comms
#define MAX_BUFF (64) // Max message length sent to Arduino

char buffer[MAX_BUFF]; // where we store the message until we get a ;
int sofar;             // How much is in the buffer 

#define VERSION (1.0)
#define MIN_FEEDRATE (50.0)
#define MAX_FEEDRATE (100.0)
float fr = 100.0;
float nfr = 50.0;
int mode_abs=1;
float px = 0.0;
float py = 0.0;
long step_delay=1000000.0/fr;

String readString = "";


/**
 * Look for character /code/ in the buffer and read the float that immediately follows it.
 * @return the value found.  If nothing is found, /val/ is returned.
 * @input code the character to look for.
 * @input val the return value if /code/ is not found.
 **/
float parsenumber(char code,float val) {
  char *ptr=buffer;  // start at the beginning of buffer
  while((long)ptr > 1 && (*ptr) && (long)ptr < (long)buffer+sofar) {  // walk to the end
    if(*ptr==code) {  // if you find code on your walk,
      return atof(ptr+1);  // convert the digits that follow into a float and return it
    }
    ptr=strchr(ptr,' ')+1;  // take a step from here to the letter after the next space
  }
  return val;  // end reached, nothing found, return default val.
}


// Start up code to run only once
void setup(){

  m1.setSpeed(10);
  m2.setSpeed(10);

  Serial.begin(BAUD);
  Serial.setTimeout(1); 

  //displayMenu(); // display menu
  setFeedrate(100); // set default speed
  signalReady();
}

// loop() runs forever
void loop(){
  // Listen for commands
  while(!Serial.available()) {} // wait for data to arrive
  if(Serial.available()){ // If something is available
    // char c = Serial.read(); // Grab the first character
    // Serial.print(c); // Print what was received

    String test = Serial.readString();
    Serial.print(test);

    for(int j = 0; j < test.length(); j++){
      char c = test[j];
      //IF there's room in the buffer, store the byte.
      //this prevents loss of data if the buffer is full.
      if(sofar < MAX_BUFF) buffer[sofar++]=c;
      // IF we got a return character (\n) then the message is done
      if(c=='\n'){
        // Serial.print(F("\r\n")); // for debugging
        // strings gotta end with a \0
        buffer[sofar]=0;
        processCommand(); // DO SOMETHING with the command
        // signalReady();
      }
    }
  }
}

/*Display menu info*/
void displayMenu(){

  Serial.print(F("2 Axis Scara Arm ")); // F stores string in prog mem instead of RAM
  Serial.println(VERSION);
  Serial.println(F("COMMANDS"));
  Serial.println(F("Rapid Positioning: G00 X{x_coord} Y{y_coord}"));
  Serial.println(F("Linear Interpolation: G01 X{x_coord} Y{y_coord} F{speed}"));
  Serial.println(F("Absolute Positioning: G90"));
  Serial.println(F("Relative Positioning: G91"));
  Serial.println(F("Set Units to Inches: G20"));
  Serial.println(F("Set Units to Millimeters: G21"));
  Serial.println(F("End the Program: M02"));
  Serial.println(F("Change Writing Utensil: M06-{tool_#}"));
  Serial.println(F("Save Current State: M70"));
  Serial.println(F("Restore Saved State: M72"));
}

/*Prepares input buffer to receive new message;
  tells serial connected device it is ready for more
*/
void signalReady(){
  sofar = 0; // Clear the input buffer
  // Serial.print(F("> ")); // Signal that we're ready to receive input
}


// INTERPRETING COMMANDS

/* Read input buffer and find any recognized commands. One G or M command per line. */
void processCommand(){
  // Serial.print("We're in processCommand");
  // Look for commands that start with a "G"
  int cmd=parsenumber('G',-1);
  switch(cmd) {
    // G00
    case 0: // move in a line
    // G01
    case 1: // move in a line
      setFeedrate(parsenumber('F', fr));
      // calling line and passing whether mode is absolute
      line( parsenumber('X', (mode_abs?px:0) +  (mode_abs?0:px)), parsenumber('Y', (mode_abs?py:0) +  (mode_abs?0:py)) );
      break;

    // G90 - absolute mode
    case 90: mode_abs=1; break;
    // G91 - relative mode
    case 91: mode_abs=0; break;

    // TO DO: fill in G20
    // TO DO: fill in G21

    default: break;
  }

  // Look for commands that start with an "M"
  cmd=parsenumber('M',-1);
  switch(cmd){

    // TO DO: M02
    // TO DO: M06
    // TO DO: M70
    // TO DO: M72
    case 18:
      // release motor 1
      m1.release();
      // relesase motor 2
      m2.release();
      Serial.print("Release the motors!");
      break;
    default: break;
  }

  // If string has no G or M commands, it will reach this place
  // and the Arduino will ignore it....
}

// MOVEMENT ALGORITHM

/*Using Bresenham's line algorithm to move both motors 
  @input newx the destination x position
  @input newy the destination y position
*/
void line(float newx, float newy){


  long dx=newx-px;
  long dy=newy-py;
  int dirx=dx > 0?1:-1; // direction to move
  int diry=dy > 0?1:-1; 
  dx=abs(dx);
  dy=abs(dy);

  long i;

  long over=0;

  if(dx > dy){
    for(i=0; i<dx; i++){
      // m1 takes one step in dirx
      m1.onestep(dirx, SINGLE);
      over+=dy;

      if(over>=dx){
        over-=dx;
        // m2 takes one step in diry
        m2.onestep(diry, SINGLE);
      }
      pause(step_delay); // step_delay is a global connected to feed rate
    }

    
    // TO DO: test limits here
  } else {

    for(i=0; i<dy; ++i){
      // m2 takes one step in diry
      m2.onestep(diry, SINGLE);
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

  Serial.print("Current x position: ");
  Serial.print(px);
  Serial.print(" Current y position: ");
  Serial.print(py);
}

/* delay for # of microseconds
   @input how many milliseconds to wait
*/
void pause(long ms){
  delay(ms/1000);
  delayMicroseconds(ms%1000);
}

/* Setting the feedrate (speed motors will move) 
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

