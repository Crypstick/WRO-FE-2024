#include <Wire.h>
#include <Evo.h>
#include <EV3Motor.h>
#include <VL53L0X.h>
#include <MPU.h>

EVO evo;
MPU gyro;
VL53L0X distance_left(2);
VL53L0X distance_right(3);







void setup() {
  Serial.begin(115200);
  evo.begin();
  evo.playTone(NOTE_A4, 300);
  evo.beginDisplay();
  evo.writeToDisplay(evo.getBattery(), 0, true);

  distance_left.begin();
  distance_right.begin();
  gyro.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print(gyro.getHeading());
  Serial.print(", left dist: ");
  Serial.print(distance_left.getDistance());
  Serial.print(", right dist: ");
  Serial.print(distance_right.getDistance());
  Serial.println(", set heading: ");
}
