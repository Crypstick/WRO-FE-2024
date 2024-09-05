#include <Wire.h>
#include <Evo.h>
#include <EV3Motor.h>
#include <VL53L0X.h>
#include <MPU.h>
#include <HUSKYLENS.h>
#include <SoftwareSerial.h>
#include "Adafruit_TCS34725.h"

#define TCA9548A_ADDR 0x70
#define BQ25887_ADDR 0x6A

#define WHITE 0
#define ORANGE 1
#define BLUE 2

EVO evo;
MPU gyro;
//VL53L0X distance_front(4);
VL53L0X distance_left(2);
VL53L0X distance_right(3);

HUSKYLENS huskylens;
SoftwareSerial mySerial(43, 44); // RX, TX REMMBER TO FLIP BACK TO 43, 44
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

void setup() {
  Serial.begin(115200);
  evo.begin();
  evo.playTone(NOTE_A4, 300);
  evo.beginDisplay();
  evo.writeToDisplay(evo.getBattery(), 0, true);

  distance_left.begin();
  distance_right.begin();
  //distance_front.begin();
  gyro.begin();

  mySerial.begin(9600);
  while (!huskylens.begin(mySerial))
  {
      Serial.println(F("Begin failed!"));
      evo.writeToDisplay(2, 2, "huskylens died");
      Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>Serial 9600)"));
      Serial.println(F("2.Please recheck the connection."));
      delay(100);
  }

  evo.selectI2CChannel(COLOR_PIN);
  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }
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
  /*
  Serial.print(gyro.getHeading());
  //Serial.print(", front dist: ");
  //Serial.print(distance_front.getDistance());
  Serial.print(", left dist: ");
  Serial.print(distance_left.getDistance());
  Serial.print(", right dist: ");
  Serial.print(distance_right.getDistance());
  Serial.print(", color: ");
  Serial.print(getColor(false));
  */
  Serial.print(", camera: ");
  if (findRelevantObject()) {
    printResult(relevantObject);
    int inWidth = relevantObject.width;
    int length = relevantObject.height;
    int area = relevantObject.height * inWidth;
    if (inWidth < 80 || area < 6000 || inWidth > length) Serial.println("tracking");
    else Serial.println("hardcode");
  }
  else Serial.println("No relevent object!");
}

void printResult(HUSKYLENSResult result){
    if (result.command == COMMAND_RETURN_BLOCK){
        Serial.println(String()+F("Block:xCenter=")+result.xCenter+F(",yCenter=")+result.yCenter+F(",width=")+result.width+F(",height=")+result.height+F(",ID=")+result.ID);
    }
    else {
        Serial.println("Object unknown!");
    }
}

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

  if (redValue > greenValue && redValue > blueValue) return ORANGE;
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
