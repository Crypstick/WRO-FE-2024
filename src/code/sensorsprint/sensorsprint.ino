
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

#define REDBLOCK 1
#define GREENBLOCK 2


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


Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

struct colorCalibration {
  unsigned int blackValue;
  unsigned int whiteValue;
};

int gammatable[256];
int RGBmap(unsigned int x, unsigned int inlow, unsigned int inhigh, int outlow, int outhigh);

// initiate three stuctures to hold calibration of Red, Green and LED's in TCS3472
colorCalibration redCal, greenCal, blueCal;



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
  redCal.whiteValue = 198;
  greenCal.blackValue = 23;
  greenCal.whiteValue = 209;
  blueCal.blackValue = 15;
  blueCal.whiteValue = 158;

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
  uint16_t r, g, b, c; // raw values of r,g,b,c as read by TCS3472
  // Variables used to hold RGB values between 0 and 255
  int redValue;
  int greenValue;
  int blueValue;
  int clearValue;
  // colorTemp = tcs.calculateColorTemperature(r, g, b);
  evo.selectI2CChannel(4);
  tcs.getRawData(&r, &g, &b, &c);
  
  // Print out raw data resulting from read cycle, use to calibrate bias value
  Serial.print("R: "); Serial.print(r); Serial.print(" ");
  Serial.print("G: "); Serial.print(g); Serial.print(" ");
  Serial.print("B: "); Serial.print(b); Serial.print(" ");
  Serial.print("C: "); Serial.print(c); Serial.print(" ");
  Serial.println(" ");

  delay(50);

  // Convert TCS3472 raw reading into value between 0 and 255 for analogwrite function
  redValue = RGBmap(r, redCal.blackValue, redCal.whiteValue, 0, 255);
  greenValue = RGBmap(g, greenCal.blackValue, greenCal.whiteValue, 0, 255);
  blueValue = RGBmap(b, blueCal.blackValue, blueCal.whiteValue, 0, 255);

  // Print out values
  Serial.print("RValue: "); Serial.print(redValue); Serial.print(" ");
  Serial.print("GValue: "); Serial.print(greenValue); Serial.print(" ");
  Serial.print("BValue: "); Serial.print(blueValue); Serial.print(" ");
  Serial.println(" ");

}

// Function to map TCS3472 values to 0 to 255
// Same as Arduino map() function - rewritten to make variables compatiable with inputs and outputs
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
