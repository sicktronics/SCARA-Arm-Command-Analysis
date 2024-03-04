#include <AFMotor.h>
#include <Servo.h>

// Setting up two stepper motors and a servo

// CHANGE: Num steps per rev
// for num steps: 360/degrees per step
AF_Stepper m1(200,1);
AF_Stepper m2(200,2);

int m1_steps = 0;
int m2_steps = 0;

// CHANGE: Starting angles
float m1_angle = 0.0;  // m1 angle wrt x axis
float m2_angle = 0.0; // m2 angle wrt len1
float deg_per_step = 1.8;

// Servo myservo;
// CHANGE: length (in mm) of each arm
float len1 = 10.0;
float len2 = 10.0;


#define VERSION (1.0) // Program version
#define MIN_FEEDRATE (50.0) // Min, max feedrate (need to update, this is just for testing)
#define MAX_FEEDRATE (100.0)
float fr = 50.0;           // Current feedrate
float nfr = 50.0;           // New feedrate
int mode_abs=1;             // Whether the current mode is absolute or relative (absolute by default)
float px = 0.0;             // x, y position
float py = 0.0;
long step_delay=1000000.0/fr; // Delay between step commands
int current = 1;            // Current utensil--by default, 1 - pen


void setup() {

  Serial.begin(57600);
  // put your setup code here, to run once:

  // myservo.attach(9);

  // m1.setSpeed(10);
  // motor1.onestep(FORWARD, SINGLE);
  m1.release();

  // delay(1000);

  // motor1.step(100, FORWARD, SINGLE);
  // motor1.step(100, BACKWARD, SINGLE);

  // m2.setSpeed(10);
  // motor2.onestep(FORWARD, SINGLE);
  m2.release();

  line(5.0, 5.0);
  // line(2.5, 2.5);

  // delay(1000);

  // motor2.step(100, FORWARD, SINGLE);
  // motor2.step(100, BACKWARD, SINGLE);

}

void loop() {

  // myservo.write(84); // this is the uncalibrated stop

  // delay(500);
  // put your main code here, to run repeatedly:
  // motor1.step(100, FORWARD, SINGLE);
  // motor2.step(100, FORWARD, SINGLE);

  // motor1.step(100, BACKWARD, SINGLE);
  // motor2.step(100, BACKWARD, SINGLE);

}

// MOVEMENT ALGORITHM

// Supporting functions
float lawOfCosines(float a, float b, float c){
  // TO DO: will this ACOS cover all of our bases?
  return acos(((a*a)+(b*b)-(c*c))/(2*a*b));
}

float distance(float x, float y){
  float d = sqrt((x*x)+(y*y));
  return d;
}

float angles_A1(float x, float y){

  float dist = distance(x, y);
  Serial.println();
  Serial.print("dist: ");
  Serial.println(dist);
  float D1 = atan2(y, x);
  Serial.print("D1: ");
  Serial.println(D1);
  float D2 = lawOfCosines(dist, len1, len2);
  Serial.print("D2: ");
  Serial.println(D2);

  float A1 = D1 + D2;
  Serial.print("A1: ");
  Serial.println(A1);
  float A2 = lawOfCosines(len1, len2, dist);

  return A1;
}

float angles_A2(float x, float y){
  float dist = distance(x, y);
  Serial.println();
  Serial.print("dist: ");
  Serial.println(dist);
  float A2 = lawOfCosines(len1, len2, dist);
  Serial.print("A2: ");
  Serial.println(A2);
  return A2;
}

// CONFIRMED: works
float deg(float rad){
  return rad*180/(PI);
}



// WILL ASSUME EVERYTHING IS ABSOLUTE FOR NOW
void line(float newx, float newy){

  // TO DO: add checks for not extending past MAX LEN

  // This gives us the angle in radians
  float a1 = angles_A1(newx, newy);
  float a2 = angles_A2(newx, newy);
  Serial.print("a1 in degrees: ");
  Serial.print(deg(a1));
  Serial.print(" a2 in degrees: ");
  Serial.print(deg(a2));

  // TO DO: add checks for no segments running into themselves

  // CASE 1: both a1 and a2 are greater than the current angles
  if(deg(a1)>m1_angle && deg(a2)>m2_angle){
    // assumes both angles have to rotate positively
    while(m1_angle<deg(a1) || m2_angle<deg(a2)){
      if(m1_angle<deg(a1)){
        m1.onestep(FORWARD, SINGLE);
        m1_steps++;
        m1_angle+=deg_per_step;
      }
      if(m2_angle<deg(a2)){
        m2.onestep(FORWARD, SINGLE);
        m2_steps++;
        m2_angle+=deg_per_step;
      }
      Serial.println(m1_angle);
      pause(step_delay); // step_delay is a global connected to feed rate
    }
  }


  // CASE 2: both a1 and a2 are less than the current angles
  if(deg(a1)<=m1_angle && deg(a2)<=m2_angle){
    // assumes both angles have to rotate positively
    while(m1_angle>=deg(a1) || m2_angle>=deg(a2)){
      if(m1_angle>=deg(a1)){
        m1.onestep(BACKWARD, SINGLE);
        m1_steps--;
        m1_angle-=deg_per_step;
      }
      if(m2_angle>=deg(a2)){
        m2.onestep(BACKWARD, SINGLE);
        m2_steps--;
        m2_angle-=deg_per_step;
      }
      pause(step_delay); // step_delay is a global connected to feed rate
    }
  }

  // CASE 3: a1 is greather than the current angle, a2 is less than the current angle
  if(deg(a1)>m1_angle && deg(a2)<=m2_angle){
    // assumes both angles have to rotate positively
    while(m1_angle<deg(a1) || m2_angle>=deg(a2)){
      if(m1_angle<deg(a1)){
        m1.onestep(FORWARD, SINGLE);
        m1_steps++;
        m1_angle+=deg_per_step;
      }
      if(m2_angle>=deg(a2)){
        m2.onestep(BACKWARD, SINGLE);
        m2_steps--;
        m2_angle-=deg_per_step;
      }
      pause(step_delay); // step_delay is a global connected to feed rate
    }
  }

  // CASE 4: a1 is less than the current angle, a2 is greater than the current angle
  if(deg(a1)<=m1_angle && deg(a2)>m2_angle){
    // assumes both angles have to rotate positively
    while(m1_angle>=deg(a1) || m2_angle<deg(a2)){
      if(m1_angle>=deg(a1)){
        m1.onestep(BACKWARD, SINGLE);
        m1_steps--;
        m1_angle-=deg_per_step;
      }
      if(m2_angle<deg(a2)){
        m2.onestep(FORWARD, SINGLE);
        m2_steps++;
        m2_angle+=deg_per_step;
      }
      // Serial.println(m2_angle);
      pause(step_delay); // step_delay is a global connected to feed rate
    }
  }


}

/* 
   Delay for # of microseconds
   @input how many milliseconds to wait
*/
void pause(long ms){
  delay(ms/1000);
  delayMicroseconds(ms%1000);
}
