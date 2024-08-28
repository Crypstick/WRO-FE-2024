#include <Wire.h>
#include <Evo.h>
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
#define BOTH 0
#define LEFT 1
#define RIGHT 2
unsigned short STARTHEADING;
const int STEERINGDIFFERENCE = 100;
const int kP = 8;

bool gyro_initialised = false;
int dir = 0;
int hit_count = 0;
const int targets[4] = {95, 176, 247, 331};



//function prototypes
int angleDifference(int a, int b);

void initialiseSteering();
void setSteering(int speed, int percent);
void correctSteering();
void sensorprint();
void followSegment(int target, int dir);
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
  hit_count = 0;
  while (hit_count < 10) {
    if ( abs(gyro.getHeading() - targets[0]) < 10 ) hit_count++;
    else hit_count = 0;
    Serial.println("waiting for gyro");
  }
  drive_motor.run(250);

}


int turns = 0;
int turningSide = RIGHT;
void loop() {

  //sensorprint();
  //wait to be vaugley in line
  evo.playTone(NOTE_A4, 100);
  while (abs(gyro.getHeading() - targets[dir]) > 10) {
    Serial.println("waiting");
    followSegment(targets[dir], turningSide);
    sensorprint();
  }
  //wait for a wall
  evo.playTone(NOTE_A4, 100);
  hit_count = 0;
  while (hit_count < 4) {
    int dist_right = distance_right.getDistance();
    if (dist_right < 800) hit_count++;
    else if (dist_right != -1000) hit_count = 0;
    Serial.println("in");
    followSegment(targets[dir], turningSide);
    sensorprint();
  }
  //wait for no wall
  evo.playTone(NOTE_A4, 100);
  hit_count = 0;
  while (hit_count < 4) {
    int dist_right = distance_right.getDistance();
    if (dist_right > 800) hit_count++;
    else if (dist_right != -1000) hit_count = 0;
    Serial.println("in");
    followSegment(targets[dir], turningSide);
    sensorprint();
  }
 
  if (dir == 3) dir = 0;
  else dir += 1;
  Serial.println("BANGGGG");
  turns++;
  drive_motor.run(0);
  delay(1000);
  drive_motor.run(250);
  if (turns == 12) {
    while (abs(gyro.getHeading() - targets[dir]) > 10) {
      Serial.println("waiting");
      followSegment(targets[dir], turningSide);
      sensorprint();
    }
    //wait for a wall
    hit_count = 0;
    while (hit_count < 4) {
      int dist_right = distance_right.getDistance();
      if (dist_right < 800) hit_count++;
      else if (dist_right != -1000) hit_count = 0;
      Serial.println("in");
      followSegment(targets[dir], turningSide);
      sensorprint();
    }
    drive_motor.run(0);
    while (true) {sensorprint();}
  }

  
  

  //followSegment(targets[0]);
}  


void followSegment(int target, int turningSide) {
  const int thres = 50;

  if (turningSide == BOTH || turningSide == LEFT) {
    if (distance_left.getDistance() < thres) {
      setSteering(250, -100);
      return;
    }
  }
  else if (turningSide == BOTH || turningSide == RIGHT) {
    if (distance_right.getDistance() < thres) {
      setSteering(250, 100);
      return;
    }
  }
  
  int p = angleDifference(gyro.getHeading(), targets[dir]) * 8;
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

