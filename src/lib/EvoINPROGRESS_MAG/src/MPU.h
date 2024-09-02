#ifndef MPU_H
#define MPU_H

#include <Evo.h>
#include <MPU9250.h>

class MPU
{
private:
    MPU9250 mpu;
    uint8_t _i2cPort;
    float _yaw = 0;
    int _targetHeading;
    uint32_t _lastms = 0;


public:
    // Constructor with port
    MPU();
    void begin();
    float getHeading(int offset = 0);
    void setTargetHeading(int target);
    int getTargetHeading();
    float getRelativeHeading();
};
#endif