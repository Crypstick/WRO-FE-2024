#include <Wire.h>
#include <Evo.h>
#include <EV3Motor.h>
#include <VL53L0X.h>
#include <MPU.h>
#include <HUSKYLENS.h>
#include <SoftwareSerial.h>
#include <Adafruit_TCS34725.h>

#define REDBLOCK 1
#define GREENBLOCK 2

#define BOTH 0
#define LEFT 1
#define RIGHT 2
#define WHITE 0
#define ORANGE 1
#define BLUE 2

EVO evo;
EV3Motor steer_motor(M3, true);
EV3Motor drive_motor(M2, false);
EV3Motor turnstile_motor(M4, false);
MPU gyro;

HUSKYLENS huskylens;
SoftwareSerial mySerial(43, 44); // RX, TX REMMBER TO FLIP BACK TO 43, 44
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_24MS, TCS34725_GAIN_4X);
const int COLOR_PIN = 4;

struct colorCalibration {
  unsigned int blackValue;
  unsigned int whiteValue;
};

const int STEERINGDIFFERENCE = 100;
const int kP = 7;
const int targets[4] = {280, 192, 105, 21};  // clockwise manner
const int driveSpeed = 150;
const int headingAligned_thres = 10;

int dir = 0;
int turns = 0;
int turningSide = BOTH;
bool aligned_once = false;
colorCalibration redCal, greenCal, blueCal;
HUSKYLENSResult relevantObject;

void initialiseSteering();
void setStartingDir();
void followSegment(int target, int turningDir);
void setSteering(int speed, int percent);


bool alignToHeading();
void turningPoint();

bool findRelevantObject();
void printResult(HUSKYLENSResult result);

bool correctColorTrigger(int turningDir);
int getColor(bool printRaw);
int RGBmap(unsigned int x, unsigned int inlow, unsigned int inhigh, int outlow, int outhigh);

void sensorprint();
int angleDifference(int a, int b);






void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  evo.begin();
  evo.playTone(NOTE_A4, 300);
  evo.beginDisplay();
  evo.writeToDisplay(evo.getBattery(), 0, true);

  drive_motor.begin();
  steer_motor.begin();
  turnstile_motor.begin();

  gyro.begin();

  //husky begin
  mySerial.begin(9600);
  while (!huskylens.begin(mySerial))
  {
      Serial.println(F("Begin failed!"));
      evo.writeToDisplay(2, 2, "huskylens died");
      Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>Serial 9600)"));
      Serial.println(F("2.Please recheck the connection."));
      delay(100);
  }

  //color sensor
  //initialiseSteering();
  evo.selectI2CChannel(4);
  delay(10);
  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }
  redCal.blackValue = 33;
  redCal.whiteValue = 235;
  greenCal.blackValue = 25;
  greenCal.whiteValue = 245;
  blueCal.blackValue = 20;
  blueCal.whiteValue = 188;

  evo.selectI2CChannel(5);
  delay(10);
  initialiseSteering();
  



  //start code
  setStartingDir();
  Serial.println(dir);

}

void loop() {
  // put your main code here, to run repeatedly:
  if (correctColorTrigger(dir) && aligned_once) {
    turningPoint();
    Serial.print("we hit color we turning");
  }
  else if (false) {

  }
  else {
    alignToHeading();
    Serial.print("aligning to heading");
  }
  delay(50);

}



bool correctColorTrigger(int turningDir) {
  if ((turningDir == LEFT || turningDir == BOTH) && (getColor(false) == BLUE)) {
    dir = LEFT;
    return true;
  }
  if ((turningDir == RIGHT || turningDir == BOTH) && (getColor(false) == ORANGE)) {
    dir = RIGHT;
    return true;
  }
  return false;
}

void turningPoint() {
  Serial.println("we turning");
  aligned_once = false;
  turns++;
  //change gyro target
  if (turningSide == LEFT) {
    if (dir == 0) dir = 3;
    else dir -= 1;
  } else if (turningSide == RIGHT) {
    if (dir == 3) dir = 0;
    else dir += 1;
  }
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

void followSegment(int target, int turningDir) {
  Serial.println("we runnning");
  evo.selectI2CChannel(5);
  //delay(10);
  drive_motor.run(driveSpeed);
  evo.selectI2CChannel(4);
  const int thres = 150;
  int p = angleDifference(targets[dir], gyro.getHeading()) * kP;
  setSteering(250, p);
}

void setSteering(int speed, int percent) {
  percent = clamp(percent, -100, 100);
  float a = ((static_cast<float>(STEERINGDIFFERENCE)) / 100) * percent;
  evo.selectI2CChannel(5);
  steer_motor.runTarget(speed, a);
  delay(80);
  evo.selectI2CChannel(4);
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

int getColor(bool printRaw) {
  uint16_t r, g, b, c; // raw values of r,g,b,c as read by TCS3472
  // Variables used to hold RGB values between 0 and 255
  int redValue;
  int greenValue;
  int blueValue;
  int clearValue;
  evo.selectI2CChannel(COLOR_PIN);
  delay(10);
  tcs.getRawData(&r, &g, &b, &c);

  redValue = RGBmap(r, redCal.blackValue, redCal.whiteValue, 0, 255);
  greenValue = RGBmap(g, greenCal.blackValue, greenCal.whiteValue, 0, 255);
  blueValue = RGBmap(b, blueCal.blackValue, blueCal.whiteValue, 0, 255);
  if (printRaw) {
    Serial.print(redValue);
    Serial.print("\t");
    Serial.print(greenValue);
    Serial.print("\t");
    Serial.print(blueValue);
    Serial.print("\t : ");
  }

  if (redValue + greenValue + blueValue > 600) return WHITE;
  else if (redValue > greenValue && redValue > blueValue) return ORANGE;
  else if (blueValue > redValue && blueValue > greenValue) return BLUE;
  else return WHITE;

}

int RGBmap(unsigned int x, unsigned int inlow, unsigned int inhigh, int outlow, int outhigh){
  float flx = float(x);
  float fla = float(outlow);
  float flb = float(outhigh);
  float flc = float(inlow);
  float fld = float(inhigh);

  float res = ((flx-flc)/(fld-flc))*(flb-fla) + fla;

  return int(res);
}

bool findRelevantObject() {

  if (!huskylens.request()) {
    
    Serial.println(F("Fail to request data from HUSKYLENS, recheck the connection!"));
    return false;
  }
  else if (!huskylens.available()) { // no block on the screen, keep to current dir
    return false;
  } else {
    int biggestY = -1;
    HUSKYLENSResult candidate;
    for (int i = 0; i < huskylens.countBlocks(); i++) {
      HUSKYLENSResult current = huskylens.getBlock(i);
      if ((current.width > 35 && current.width < 160) && (current.width < current.height || current.height * current.width > 6000)){
        if (current.yCenter > biggestY) {
          biggestY = current.yCenter;
          candidate = current;
        }
      }
    }
    if (biggestY != -1){
      relevantObject = candidate;
      return true;
    }
    else return false;
  }
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

void printResult(HUSKYLENSResult result){
    if (result.command == COMMAND_RETURN_BLOCK){
        Serial.println(String()+F("Block:xCenter=")+result.xCenter+F(",yCenter=")+result.yCenter+F(",width=")+result.width+F(",height=")+result.height+F(",ID=")+result.ID);
    }
    else {
        Serial.println("Object unknown!");
    }
}

void sensorprint() {
  Serial.print(gyro.getHeading());
  Serial.print(", color: ");
  Serial.print(getColor(false));
  Serial.print(", camera: ");
  if (findRelevantObject()) {
    printResult(relevantObject);
    int inWidth = relevantObject.width;
    int length = relevantObject.height;
    int area = relevantObject.height * inWidth;
    if (inWidth < 80 || area < 6000 || inWidth > length) Serial.println("tracking");
    else Serial.println("hardcode");
  }
  else {
    Serial.println("No relevent object!");
  }
}