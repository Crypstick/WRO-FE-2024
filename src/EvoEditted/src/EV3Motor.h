#ifndef EV3MOTOR_H
#define EV3MOTOR_H
#include <EVO.h>
#include <Wire.h>
#include <SparkFunSX1509.h> //Click here for the library: http://librarymanager/All#SparkFun_SX1509
#include <ESP32Encoder.h>

struct MotorPins
{
    uint8_t power1;
    uint8_t power2;
    uint8_t tach1;
    uint8_t tach2;
};

class EV3Motor
{
private:
    int _motorPort;
    bool _motorFlip;
    int currentSpeed;
    int currentAngle;
    MotorPins motorPins;
    ESP32Encoder encoder;
    static SX1509 io;
    static bool SX1509Initialized;

public:
    // Constructor with port and positive direction
    EV3Motor(MotorPort motorPort, bool motorFlip = false);
    void begin();

    // Method to get current angle
    int getAngle();

    // Method to reset angle
    void resetAngle();

    // Method to stop the motor
    void coast();

    // Method to brake the motor
    void brake();

    // Method to run the motor at a specified speed
    void run(int speed);

    // Method to run the motor for a specified number of degrees
    void runDegrees(int speed, int degrees);
};

#endif