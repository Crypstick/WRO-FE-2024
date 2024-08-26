#include <Wire.h>
#include <Evo.h>
#include <Magnometer.h>
#include <EV3Motor.h>
#include <VL53L0X.h>
#include <MPU.h>

//Declare objects
EVO evo;
EV3Motor steer_motor(M1, true);
EV3Motor drive_motor(M2, false);
EV3Motor turnstile_motor(M4, false);

MPU gyro;
VL53L0X distance_left(2);
VL53L0X distance_right(3);

//Universal variables
unsigned short STARTHEADING;
const int STEERINGDIFFERENCE = 100;

bool gyro_initialised = false;
int _yaw = 0;
long _lastms = 0;
int dir = 0;
const int targets[4] = {25, 100, 210, 300};



//function prototypes
int angleDifference(int a, int b);

void initialiseSteering();
void setSteering(int speed, int percent);
void correctSteering();
void sensorprint();
void adjustHeading(int target);
void print_calibration();
void print_roll_pitch_yaw();

void setup() {
  Serial.begin(115200);
  //Initializes the Evolution X1
  evo.begin();
  evo.playTone(NOTE_A4, 300);
  evo.beginDisplay();
  evo.writeToDisplay(evo.getBattery(), 0, true);

  steer_motor.begin();
  drive_motor.begin();
  turnstile_motor.begin();

  distance_left.begin();
  distance_right.begin();
  gyro.begin();
 

  initialiseSteering();
  //initialiseTurnstile();


  //Plays the buzzer for 300ms
  evo.playTone(NOTE_G4, 300);
  while (abs(gyro.getHeading() - targets[0]) > 10) {}
  drive_motor.run(250);

}



void loop() {

  //sensorprint();
  while (abs(gyro.getHeading() - targets[dir]) > 10) {
    Serial.println("waiting");
    adjustHeading(targets[dir]);
    sensorprint();
  }
  while (distance_right.getDistance() < 500) {
    Serial.println("in");
    adjustHeading(targets[dir]);
    sensorprint();
  }
  if (dir == 1) {
    drive_motor.run(0);
    while (true) {sensorprint();}
  }
  if (dir == 3) dir = 0;
  else dir += 1;

  
  

  //adjustHeading(targets[0]);
}  


void adjustHeading(int target) {
  int p = angleDifference(gyro.getHeading(), targets[dir]) * 6;
  setSteering(250, p);
}

void sensorprint() {
  Serial.print(gyro.getHeading());
  Serial.print(", left dist: ");
  Serial.print(distance_left.getDistance());
  Serial.print(", right dist: ");
  Serial.print(distance_right.getDistance());
  Serial.println(", set heading: ");
}

void initialiseTurnstile() {
  turnstile_motor.resetAngle();
  turnstile_motor.run(-100);
  delay(1000);
  turnstile_motor.resetAngle();
  turnstile_motor.run(100);
  delay(1000);
  int a = turnstile_motor.getAngle() / 2;
  turnstile_motor.run(-70);
  while (turnstile_motor.getAngle() > a) {}
  turnstile_motor.resetAngle();
  turnstile_motor.coast();
}


void initialiseSteering() {
  steer_motor.resetAngle();
  steer_motor.run(-200);
  delay(1000);
  steer_motor.resetAngle();
  steer_motor.run(200);
  delay(1000);
  int a = steer_motor.getAngle() / 2;
  steer_motor.run(-150);
  while (steer_motor.getAngle() > a) {}
  steer_motor.resetAngle();
  steer_motor.coast();
}

void setSteering(int speed, int percent) {
  percent = clamp(percent, -100, 100);
  float a = ((static_cast<float>(STEERINGDIFFERENCE)) / 100) * percent;
  Serial.println(a);
  steer_motor.runTarget(speed, a);
}

int angleDifference(int a, int b) {
  int noseam = (::fabs(a - b)) * -1;
  int seam = 360 - std::max(a, b) + std::min(a, b);
  int dir = (b - a) / (::fabs(b - a));
  if (seam < ::fabs(noseam)) {
    return seam * dir;
  } else {
    return noseam * dir;
  }
}

int clamp(int a, int lower, int upper) {
  if (a < lower) return lower;
  if (a > upper) return upper;
  return a;
}

