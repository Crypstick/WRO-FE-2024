#ifndef MAGNOMETER_H
#define MAGNOMETER_H
#include <EVO.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>

class Magnometer
{
private:
    Adafruit_HMC5883_Unified magnometer;
    uint8_t _i2cPort;
    float _declinationAngle, _targetHeading;

public:
    // Constructor with port and positive direction
    Magnometer(uint8_t i2cPort, float declinationAngle = 0);
    void begin();
    float getHeading(float offsetHeading = 0);
    void setTargetHeading(float target);
    float getTargetHeading();
    float getRelativeHeading();
};
#endif