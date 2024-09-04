
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


Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_614MS, TCS34725_GAIN_1X);



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

}

void loop() {
  uint16_t r, g, b, c, colorTemp, lux;

  evo.selectI2CChannel(4);
  tcs.getRawData(&r, &g, &b, &c);
  // colorTemp = tcs.calculateColorTemperature(r, g, b);
  colorTemp = tcs.calculateColorTemperature_dn40(r, g, b, c);
  lux = tcs.calculateLux(r, g, b);

  Serial.print("Color Temp: "); Serial.print(colorTemp, DEC); Serial.print(" K - ");
  Serial.print("Lux: "); Serial.print(lux, DEC); Serial.print(" - ");
  Serial.print("R: "); Serial.print(r, DEC); Serial.print(" ");
  Serial.print("G: "); Serial.print(g, DEC); Serial.print(" ");
  Serial.print("B: "); Serial.print(b, DEC); Serial.print(" ");
  Serial.print("C: "); Serial.print(c, DEC); Serial.print(" ");
  Serial.println(" ");
}

void i2c_generic_write(uint8_t const i2c_slave_addr, uint8_t const reg_addr, uint8_t const * buf, uint8_t const num_bytes)
{
  Wire.beginTransmission(i2c_slave_addr);
  Wire.write(reg_addr);
  for(uint8_t bytes_written = 0; bytes_written < num_bytes; bytes_written++) {
    Wire.write(buf[bytes_written]);
  }
  Wire.endTransmission();
}

void i2c_generic_read(uint8_t const i2c_slave_addr, uint8_t const reg_addr, uint8_t * buf, uint8_t const num_bytes)
{
  Wire.beginTransmission(i2c_slave_addr);
  Wire.write(reg_addr);
  Wire.endTransmission();

  Wire.requestFrom(i2c_slave_addr, num_bytes);
  for(uint8_t bytes_read = 0; (bytes_read < num_bytes) && Wire.available(); bytes_read++) {
    buf[bytes_read] = Wire.read();
  }
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
