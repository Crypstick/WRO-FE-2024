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
VL53L0X distance_left(1);
VL53L0X distance_right(4);

//Universal variables
#define WAIT_FOR_ALIGNMENT 0
#define WAIT_FOR_WALL 1
#define FORCED_GYRO 2
#define WAIT_FOR_NO_WALL 3
#define OPEN 0
#define OBSTACLE 1
#define BOTH 0
#define LEFT 1
#define RIGHT 2

const int STEERINGDIFFERENCE = 100;
const int kP = 8;
const bool gameMode = OPEN;
const int targets[4] = {90, 7, 270, 187};  // clockwise manner

long hit_count = 0;
int dir = 0;
int last_noWall = 0;
int turns = 0;
int status = WAIT_FOR_ALIGNMENT;
int turningSide = BOTH;




//function prototypes

void initialiseSteering();
void setSteering(int speed, int percent);
void setStartingDir();

void openPortion();
void followSegment(int target, int dir);
int findGotWall(int turnin);
int findNoWall(int dir);

void sensorprint();
void correctSteering();
int angleDifference(int a, int b);


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

  setStartingDir();
  Serial.println(dir);
  delay(2000);
  drive_motor.run(250);
}

//--------------------------------------------LOOOP-------------------------------------

void loop() {
  Serial.print("dir: ");
  Serial.print(dir);
  Serial.print(", status: ");
  Serial.print(status);
  Serial.print(", turningSide: ");
  Serial.print(turningSide);
  Serial.print(", last_noWall: ");
  Serial.print(last_noWall);
  openPortion();
}
//--------------------------------------------LOOOP-------------------------------------



void openPortion() {
  //dependent on the following global variables:
  // gameMode, hit_count, status, turns, turningSide, last_noWall, dir
  switch (status) {
    case WAIT_FOR_ALIGNMENT:
    {
      followSegment(targets[dir], turningSide);
      sensorprint();
      Serial.println("WAIT_FOR_ALIGNMENT");
      if (abs(gyro.getHeading() - targets[dir]) < 10) {
        status = WAIT_FOR_WALL;
        hit_count = 0;
        evo.playTone(NOTE_A4, 100);
      }
      break;
    }

    case WAIT_FOR_WALL:
    {
      int result = findGotWall(turningSide);
      if (result == 1) hit_count++;
      else if (result == -1) hit_count = 0;
      followSegment(targets[dir], turningSide);
      sensorprint();
      Serial.println("WAIT_FOR_WALL");

      if (hit_count > 4) {
        status = FORCED_GYRO;
        hit_count = millis();
        evo.playTone(NOTE_A4, 100);
      }
      break;
    }

    case FORCED_GYRO:
    {
      followSegment(targets[dir], turningSide);
      sensorprint();
      Serial.println("FORCED_GYRO");

      if (hit_count + 750 < millis()) {
        status = WAIT_FOR_NO_WALL;
        hit_count = 0;
        evo.playTone(NOTE_A4, 100);
        if (gameMode == OPEN) {  // end of open round???
          if (turns == 12) {
            drive_motor.run(0);
            evo.playTone(NOTE_B3, 500);
            while (true) sensorprint();
          }
        }
      }
      break;
    }

    case WAIT_FOR_NO_WALL:
    {
      int result = findNoWall(turningSide);
      if (result == 1) hit_count++;
      else if (result == -1) hit_count = 0;
      followSegment(targets[dir], turningSide);
      sensorprint();
      Serial.println("WAIT_FOR_NO_WALL");
      if (hit_count > 4) {
        status = WAIT_FOR_ALIGNMENT;
        hit_count = 0;
        evo.playTone(NOTE_A5, 100);

        turns++;
        //change gyro target
        if (turningSide == BOTH) turningSide = last_noWall;
        if (turningSide == LEFT) {
          if (dir == 0) dir = 3;
          else dir -= 1;
        } else if (turningSide == RIGHT) {
          if (dir == 3) dir = 0;
          else dir += 1;
        }
      }
      break;
    }
  }
}


void setStartingDir() {
  int suspect = -1;
  int hits = 0;
  int current, closest;

  while (hits < 10) {
    current = -1;
    closest = 100000;
    for (int i = 0; i < 4; i++) {
      int diff = abs(angleDifference(gyro.getHeading(), targets[i]));
      if (diff < closest) {
        closest = diff;
        current = i;
      }
    }

    if (suspect == current) hits++;
    else hits = 0;
    suspect = current;
  }
  dir = suspect;
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
  if (turningDir == LEFT || (turningDir == BOTH && (last_noWall == 0 || last_noWall == LEFT)) ) {
    int dist = distance_left.getDistance();
    if (dist > 800) {
      last_noWall = LEFT;
      return 1;
    } else if (dist != -1000) {
      if (last_noWall != 0) {
        last_noWall = 0;
        return -1;
      }
    }
  }
  if (turningDir == RIGHT || (turningDir == BOTH && (last_noWall == 0 || last_noWall == RIGHT)) ) {
    int dist = distance_right.getDistance();
    Serial.println(dist);
    if (dist > 800) {
      last_noWall = RIGHT;
      return 1;
    } else if (dist != -1000) {
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
  } 
  if (turningDir == BOTH || turningDir == RIGHT) {
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
