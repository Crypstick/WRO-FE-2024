
#include <Wire.h>
#include <Evo.h>
#include <EV3Motor.h>
#include <VL53L0X.h>
#include <MPU.h>
#include "Adafruit_TCS34725.h"

#include <SoftwareSerial.h>
#include <HUSKYLENS.h>


#define TCA9548A_ADDR 0x70
#define BQ25887_ADDR 0x6A

#define WHITE 0
#define ORANGE 1
#define BLUE 2

#define REDBLOCK 1
#define GREENBLOCK 2


HUSKYLENS huskylens;
SoftwareSerial mySerial(43, 44);  // RX, TX REMMBER TO FLIP BACK TO 43, 44
EVO evo;
EV3Motor steer_motor(M1, true);
EV3Motor drive_motor(M2, false);
EV3Motor turnstile_motor(M4, false);
MPU gyro;

//VL53L0X distance_front(4);
VL53L0X distance_left(2);
VL53L0X distance_right(3);
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

const int COLOR_PIN = 4;

int gammatable[256];
int getColor(bool printRaw);
int RGBmap(unsigned int x, unsigned int inlow, unsigned int inhigh, int outlow, int outhigh);
struct colorCalibration {
  unsigned int blackValue;
  unsigned int whiteValue;
};

colorCalibration redCal, greenCal, blueCal;



HUSKYLENSResult relevantObject;


bool findRelevantObject();
void printResult(HUSKYLENSResult result);
// initiate three stuctures to hold calibration of Red, Green and LED's in TCS3472



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


  evo.selectI2CChannel(COLOR_PIN);

  evo.selectI2CChannel(4);

  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }

  Serial.println("TCS3472 OK");

    // These values result from calibration of the TCS3472 looking at a black sample and a white sample
  // Must be edited here once calibration values have been determined
  redCal.blackValue = 33;
  redCal.whiteValue = 235;
  greenCal.blackValue = 25;
  greenCal.whiteValue = 245;
  blueCal.blackValue = 20;
  blueCal.whiteValue = 188;

  /* white
  Color Temp: 4431 K - Lux: 149 - R: 198 G: 209 B: 158 C: 591  
  // black
  Color Temp: 3122 K - Lux: 14 - R: 33 G: 23 B: 15 C: 71  
  */
  // Gamma function is Out = In^2.5
  // Required to correct for human vision
  // Store values in an array
  // normalized for 0 to 255

  for (int i=0; i<256; i++) {
    float x = i;
    x /= 255;
    x = pow(x, 2.5);
    x *= 255;

    gammatable[i] = int(x);

  }
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(gyro.getHeading());
  // Serial.print(", front dist: ");
  // Serial.print(distance_front.getDistance());
  /*
  Serial.print(", left dist: ");
  Serial.println(distance_left.getDistance());
  Serial.print(", right dist: ");
  Serial.println(distance_right.getDistance());
  Serial.print(", color: ");
  */
  Serial.println(getColor(true));
  
  Serial.print(", camera: ");
  if (findRelevantObject()) {
    printResult(relevantObject);
    int inWidth = relevantObject.width;
    int length = relevantObject.height;
    int area = relevantObject.height * inWidth;
    if (inWidth < 80 || area < 6000 || inWidth > length) Serial.println("tracking");
    else Serial.println("hardcode");
  } else { Serial.println("No relevent object!");
  }
} 

// 90 7 270 187

int getColor(bool printRaw) {
  uint16_t r, g, b, c; // raw values of r,g,b,c as read by TCS3472
  // Variables used to hold RGB values between 0 and 255
  int redValue;
  int greenValue;
  int blueValue;
  int clearValue;
  evo.selectI2CChannel(COLOR_PIN);
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
  if (redValue > greenValue && redValue > blueValue) return ORANGE;
  else if (blueValue > redValue && blueValue > greenValue) return BLUE;
  else return ORANGE;

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
      if (current.width > 35 && current.width < 160) {
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

void printResult(HUSKYLENSResult result) {
  if (result.command == COMMAND_RETURN_BLOCK) {
    Serial.println(String() + F("Block:xCenter=") + result.xCenter + F(",yCenter=") + result.yCenter + F(",width=") + result.width + F(",height=") + result.height + F(",ID=") + result.ID);
  } else {
    Serial.println("Object unknown!");
  }
}