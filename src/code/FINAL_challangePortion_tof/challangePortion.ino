#include <Wire.h>
#include <Evo.h>
#include <EV3Motor.h>
#include <VL53L0X.h>
#include <MPU.h>

#include <SoftwareSerial.h>
#include <HUSKYLENS.h>

#define TCA9548A_ADDR 0x70
#define BQ25887_ADDR 0x6A

#define REDBLOCK 1
#define GREENBLOCK 2

//Declare objects
HUSKYLENS huskylens;
SoftwareSerial mySerial(43, 44);  // RX, TX REMMBER TO FLIP BACK TO 43, 44
EVO evo;
EV3Motor steer_motor(M1, true);
EV3Motor drive_motor(M2, false);
EV3Motor turnstile_motor(M4, false);
MPU gyro;
//VL53L0X distance_front(1);
VL53L0X distance_left(2);
VL53L0X distance_right(3);


HUSKYLENSResult relevantObject;

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
#define WHITE 0
#define ORANGE 1
#define BLUE 2

const int STEERINGDIFFERENCE = 100;
const int kP = 7;
const int targets[4] = { 95, 8, 271, 181 };  // clockwise manner
const int driveSpeed = 150;
const int headingAligned_thres = 60;


long hit_count = 0;
int dir = 0;
int last_noWall = 0;
int turns = 0;
int turningSide = BOTH;
int blocks_passed = 2;
int nowall_hitCount = 0;
bool aligned_once = false;
int gammatable[256];




//function prototypes

void initialiseSteering();
void setSteering(int speed, int percent);
void setStartingDir();

bool findRelevantObject();
bool alignToHeading();
void waitForWall();

void openPortion();
void followSegment(int target, int dir);
int findGotWall(int turnin);
int findNoWall(int dir);

void sensorprint();
void correctSteering();
int angleDifference(int a, int b);

void printResult(HUSKYLENSResult result);




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
  //distance_front.begin();
  gyro.begin();

  initialiseSteering();
  //initialiseTurnstile();

  //Plays the buzzer for 300ms
  evo.playTone(NOTE_G4, 300);

  mySerial.begin(9600);
  while (!huskylens.begin(mySerial)) {
    Serial.println(F("Begin failed!"));
    evo.writeToDisplay(2, 2, "huskylens died");
    Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>Serial 9600)"));
    Serial.println(F("2.Please recheck the connection."));
    delay(100);
  }


  setStartingDir();
  Serial.println(dir);
  delay(2000);
  int t = millis();
  while (t + 500 > millis()) {
    sensorprint();
  }
}

//--------------------------------------------LOOOP-------------------------------------

void loop() {
  /*
  if ((abs(gyro.getHeading() - targets[dir]) < headingAligned_thres) && findNoWall(turningSide) == 1) {
    turningPoint();
    return;
  } 
  */
  if (findRelevantObject() && blocks_passed < 2 && aligned_once) {

    int inWidth = relevantObject.width;
    int length = relevantObject.height;
    int area = relevantObject.height * inWidth;
    if (relevantObject.ID == REDBLOCK) Serial.print("detected red, width ");
    if (relevantObject.ID == GREENBLOCK) Serial.print("detected green, width: ");
    Serial.print(inWidth);
    Serial.print(", length: ");
    Serial.print(length);
    Serial.print(", area: ");
    Serial.println(area);

    // start of reposition code till block near the middle of screen AND  close enough while also not being a false reading of the orange line
    if (inWidth < 80 || area < 6000 || inWidth > length) {  //if the block is too far away ( centre of block - actual centre < threshold )
      if (relevantObject.command == COMMAND_RETURN_BLOCK) {
        setSteering(250, (relevantObject.xCenter - 160) * -1.5);
        drive_motor.run(driveSpeed);
        Serial.print("Difference: ");
        Serial.println(relevantObject.xCenter - 160);
      }
    } else {
      // start of going around it code
      blocks_passed++;
      Serial.println("hardcoded section");
      if (relevantObject.ID == REDBLOCK) {  // red block
        setSteering(255, -100);
        delay(200);
        drive_motor.runDegrees(255, 800);
        setSteering(255, 100);
        delay(200);
        drive_motor.runDegrees(255, 1200);
        setSteering(255, 0);
        inWidth = 0;
      } else if (relevantObject.ID == GREENBLOCK) {  // green block code
        setSteering(255, 100);
        delay(200);
        drive_motor.runDegrees(255, 900);
        setSteering(255, -100);
        delay(200);
        drive_motor.runDegrees(255, 1200);
        setSteering(255, 0);
        inWidth = 0;
      }
    }

  } else {
    //Serial.println(F("No block or arrow appears on the screen!"));
    if (alignToHeading()) {
      if (blocks_passed > 0) {
        int wallness = findNoWall(turningSide);
        if (wallness == 1) nowall_hitCount++;
        else if (wallness == -1) nowall_hitCount = 0;
        if (nowall_hitCount > 3) turningPoint();  //turning
        else Serial.println("theres no no-wall");
      } else {
        Serial.println("no block passed yet");
      }
    } else {
      Serial.print("Not aligned to heading: ");
      Serial.println(targets[dir]);
    }
  }
  Serial.println();
  delay(50);
}
//--------------------------------------------LOOOP-------------------------------------

void turningPoint() {
  Serial.println("we turning");
  //delay(5000);
  aligned_once = false;
  blocks_passed = 0;
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
  nowall_hitCount = 0;
  delay(900);
  /*
  while (!alignToHeading()) {}
  setSteering(250, 0);
  drive_motor.runDegrees(-200, 1000);*/
}

bool findRelevantObject() {

  if (!huskylens.request()) {

    Serial.println(F("Fail to request data from HUSKYLENS, recheck the connection!"));
    return false;
  } else if (!huskylens.available()) {  // no block on the screen, keep to current dir
    return false;
  } else {
    int biggestY = -1;
    HUSKYLENSResult candidate;
    for (int i = 0; i < huskylens.countBlocks(); i++) {
      HUSKYLENSResult current = huskylens.getBlock(i);
      if ((current.width > 35 && current.width < 160) && (current.width < current.height || current.height * current.width > 6000)) {
        if (current.yCenter > biggestY) {
          biggestY = current.yCenter;
          candidate = current;
        }
      }
    }
    if (biggestY != -1) {
      relevantObject = candidate;
      return true;
    } else return false;
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

bool alignToHeading() {
  followSegment(targets[dir], turningSide);
  sensorprint();
  if (abs(gyro.getHeading() - targets[dir]) < headingAligned_thres) {
    //Serial.println("ALIGNED");
    aligned_once = true;
    return true;
  }
  return false;
  //Serial.println("WAIT_FOR_ALIGNMENT");
}

int findNoWall(int turningDir) {
  if (turningDir == LEFT || (turningDir == BOTH && (last_noWall == 0 || last_noWall == LEFT))) {
    int dist = distance_left.getDistance();
    if (dist > 900) {
      last_noWall = LEFT;
      return 1;
    } else if (dist != -1000) {
      if (last_noWall != 0) {
        last_noWall = 0;
        return -1;
      }
    }
  }
  if (turningDir == RIGHT || (turningDir == BOTH && (last_noWall == 0 || last_noWall == RIGHT))) {
    int dist = distance_right.getDistance();
    if (dist > 900) {
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
  drive_motor.run(driveSpeed);
  const int thres = 150;
  /*
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
  */

  int p = angleDifference(targets[dir], gyro.getHeading()) * kP;
  setSteering(250, p);
}

void sensorprint() {
  Serial.print(gyro.getHeading());
  Serial.print(", left dist: ");
  Serial.print(distance_left.getDistance());
  Serial.print(", right dist: ");
  Serial.print(distance_right.getDistance());
  Serial.print(", current dir: ");
  Serial.print(dir);
  Serial.print(", blocks passed: ");
  Serial.println(blocks_passed);
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

void printResult(HUSKYLENSResult result) {
  if (result.command == COMMAND_RETURN_BLOCK) {
    Serial.println(String() + F("Block:xCenter=") + result.xCenter + F(",yCenter=") + result.yCenter + F(",width=") + result.width + F(",height=") + result.height + F(",ID=") + result.ID);
  } else {
    Serial.println("Object unknown!");
  }
}

int clamp(int a, int lower, int upper) {
  if (a < lower) return lower;
  if (a > upper) return upper;
  return a;
}
