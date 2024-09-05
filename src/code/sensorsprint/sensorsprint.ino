#include <Wire.h>
#include <Evo.h>
#include <EV3Motor.h>
#include <VL53L0X.h>
#include <MPU.h>
#include <HUSKYLENS.h>
#include <SoftwareSerial.h>

#define TCA9548A_ADDR 0x70
#define BQ25887_ADDR 0x6A

EVO evo;
MPU gyro;
VL53L0X distance_front(4);
VL53L0X distance_left(2);
VL53L0X distance_right(3);

HUSKYLENS huskylens;
SoftwareSerial mySerial(43, 44); // RX, TX REMMBER TO FLIP BACK TO 43, 44






HUSKYLENSResult relevantObject;

void setup() {
  Serial.begin(115200);
  evo.begin();
  evo.playTone(NOTE_A4, 300);
  evo.beginDisplay();
  evo.writeToDisplay(evo.getBattery(), 0, true);

  distance_left.begin();
  distance_right.begin();
  distance_front.begin();
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
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print(gyro.getHeading());
  Serial.print(", front dist: ");
  Serial.print(distance_front.getDistance());
  Serial.print(", left dist: ");
  Serial.print(distance_left.getDistance());
  Serial.print(", right dist: ");
  Serial.print(distance_right.getDistance());
  Serial.print(", camera: ");
  if (findRelevantObject()) printResult(relevantObject);
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
      if (current.width > 35 && current.width < 160){
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
