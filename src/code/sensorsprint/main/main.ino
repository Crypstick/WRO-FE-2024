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
int last_noWall = 0;
const int targets[4] = {15, 285, 190, 100 };



//function prototypes
int angleDifference(int a, int b);

void initialiseSteering();
void setSteering(int speed, int percent);
void correctSteering();


void sensorprint();
void followSegment(int target, int dir);
int findGotWall(int turnin);
int findNoWall(int dir);


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
    if (abs(gyro.getHeading() - targets[0]) < 10) hit_count++;
    else hit_count = 0;
    Serial.println("waiting for gyro");
    sensorprint();
  }
  drive_motor.run(250);
}


int turns = 0;
int turningSide = BOTH;
void loop() {
  //wait to be vaugley in line
  evo.playTone(NOTE_A4, 100);
  while (abs(gyro.getHeading() - targets[dir]) > 10) {
    followSegment(targets[dir], turningSide);
    sensorprint();
    Serial.println("waiting");
  }
  //wait for a wall
  evo.playTone(NOTE_A4, 100);
  hit_count = 0;
  while (hit_count < 4) {
    int result = findGotWall(turningSide);
    if (result == 1) hit_count++;
    else if (result == -1) hit_count = 0;
    followSegment(targets[dir], turningSide);
    sensorprint();
    Serial.println("in");
  }
  //force for 0.75s
  evo.playTone(NOTE_A4, 100);
  long t = millis();
  while (t + 750 > millis()) {
    followSegment(targets[dir], turningSide);
    sensorprint();
    Serial.println("forced time");
  }
  //wait for no wall
  evo.playTone(NOTE_A4, 100);
  hit_count = 0;
  while (hit_count < 4) {
    int result = findNoWall(turningSide);
    if (result == 1) hit_count++;
    else if (result == -1) hit_count = 0;
    followSegment(targets[dir], turningSide);
    Serial.println(last_noWall);
    //sensorprint();
    //Serial.println("in");
  }

  if (turningSide == BOTH) turningSide = last_noWall;
  if (turningSide == LEFT) {
    if (dir == 0) dir = 3;
    else dir -= 1;
  } else if (turningSide == RIGHT){
    if (dir == 3) dir = 0;
    else dir += 1;
  }
  
  Serial.println("BANGGGG");
  Serial.println(targets[dir]);
  turns++;
  evo.playTone(NOTE_A5, 100);
  //drive_motor.run(0); delay(1000); drive_motor.run(250);
  if (turns == 12) {
    //wait to be vaugley in line
    evo.playTone(NOTE_A4, 100);
    while (abs(gyro.getHeading() - targets[dir]) > 10) {
      followSegment(targets[dir], turningSide);
      sensorprint();
      Serial.println("waiting");
    }
    //wait for a wall
    evo.playTone(NOTE_A4, 100);
    hit_count = 0;
    while (hit_count < 4) {
      int result = findGotWall(turningSide);
      if (result == 1) hit_count++;
      else if (result == -1) hit_count = 0;
      followSegment(targets[dir], turningSide);
      sensorprint();
      Serial.println("in");
    }
    //force for 0.75s
    evo.playTone(NOTE_A4, 100);
    long t = millis();
    while (t + 750 > millis()) {
      followSegment(targets[dir], turningSide);
      sensorprint();
      Serial.println("forced time");
    }
    drive_motor.run(0);
    while (true) { sensorprint(); }
  }




  //followSegment(targets[0]);
}
int findGotWall(int turningDir) {
  if (turningDir == BOTH || turningDir == LEFT) {
    int dist = distance_left.getDistance();
    if (dist < 800) return 1;
    else if (dist != -1000) return -1;
  }
  if (turningDir == BOTH || turningDir == RIGHT) {
    int dist = distance_right.getDistance();
    if (dist < 800) return 1;
    else if (dist != -1000) return -1;
  }
}

int findNoWall(int turningDir) {
  if (turningDir == LEFT || last_noWall == 0 || last_noWall == LEFT) {
    int dist = distance_left.getDistance();
    if (dist > 800) {
      last_noWall = LEFT;
      return 1;
    }
    else if (dist != -1000) {
      if (last_noWall != 0) {
        last_noWall = 0;
        return -1;
      }
    }
  }
  if (turningDir == RIGHT || last_noWall == 0 || last_noWall == RIGHT) {
    int dist = distance_right.getDistance();
    Serial.println(dist);
    if (dist > 800) {
      last_noWall = RIGHT;
      return 1;
    }
    else if (dist != -1000) {
      if (last_noWall != 0) {
        last_noWall = 0;
        return -1;
      }
    }
  }
}

void followSegment(int target, int turningDir) {
  const int thres = 150;

  if (turningDir == BOTH || turningDir == LEFT) {
    if (distance_left.getDistance() < thres) {
      setSteering(250, -100);
      return;
    }
  } else if (turningDir == BOTH || turningDir == RIGHT) {
    if (distance_right.getDistance() < thres) {
      setSteering(250, 100);
      return;
    }
  }

  int p = angleDifference(targets[dir], gyro.getHeading()) * 5;
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
