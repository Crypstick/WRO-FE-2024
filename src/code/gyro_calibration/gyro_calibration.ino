#include "MPU9250.h"
#include <Wire.h>
#include <Evo.h>
#include <EV3Motor.h>
#include <VL53L0X.h>
#include <cmath>


EVO evo;
MPU9250 IMU(Wire1,0x68);

VL53L0X distance_left(2);
VL53L0X distance_right(3);

int status;

void setup() {
  Serial.begin(115200);
  Wire1.begin(17, 18);
  evo.begin();
  evo.playTone(NOTE_A4, 300);
  evo.beginDisplay();
  evo.writeToDisplay(evo.getBattery(), 0, true);

  distance_left.begin();
  distance_right.begin();
  while(!Serial) {}

  // start communication with IMU 
  status = IMU.calibrateMag();
/*
  float hxb = 17.168816; // mag bias of 10 uT
  float hxs = 0.786952; // mag scale factor of 0.97
  IMU.setMagCalX(hxb,hxs);

  float hyb = 28.570538; // mag bias of 10 uT
  float hys = 0.797671; // mag scale factor of 0.97
  IMU.setMagCalY(hyb,hys);

  float hzb = -15.896074; // mag bias of 10 uT
  float hzs = 2.102503; // mag scale factor of 0.97
  IMU.setMagCalZ(hzb,hzs);
*/


  if (status < 0) {
    Serial.println("IMU initialization unsuccessful");
    Serial.println("Check IMU wiring or try cycling power");
    Serial.print("Status: ");
    Serial.println(status);
    while(1) {}
  }
  // setting the accelerometer full scale range to +/-8G 
  IMU.setAccelRange(MPU9250::ACCEL_RANGE_8G);
  // setting the gyroscope full scale range to +/-500 deg/s
  IMU.setGyroRange(MPU9250::GYRO_RANGE_500DPS);
  // setting DLPF bandwidth to 20 Hz
  IMU.setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_20HZ);
  // setting SRD to 19 for a 50 Hz update rate
  IMU.setSrd(19);
  
}

void loop() {
  // read the sensor
  IMU.readSensor();


  // display the data
  Serial.print(std::atan2(IMU.getMagY_uT(), IMU.getMagX_uT()) * 180 / PI);
  Serial.print(", mag bias X, Y, Z:");
  Serial.print(IMU.getMagBiasX_uT(),6);
  Serial.print(IMU.getMagBiasY_uT(),6);
  Serial.print(IMU.getMagBiasX_uT(),6);
  Serial.print(", mag scale x, y, z:");
  Serial.print(IMU.getMagScaleFactorX(),6);
  Serial.print(IMU.getMagScaleFactorY(),6);
  Serial.print(IMU.getMagScaleFactorZ(),6);
  delay(20);

}

// 17.423672	28.170910	-15.359415
// 