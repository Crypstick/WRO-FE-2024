#include <EV3Motor.h>

SX1509 EV3Motor::io;
bool EV3Motor::SX1509Initialized = false;

int clamp(int value, int min, int max)
{
    if (value < min)
    {
        return min;
    }
    else if (value > max)
    {
        return max;
    }
    else
    {
        return value;
    }
}

EV3Motor::EV3Motor(MotorPort motorPort, bool motorFlip)
{
    _motorPort = motorPort;
    _motorFlip = motorFlip;
    switch (_motorPort)
    {
    case M1:
        if (!_motorFlip)
            motorPins = {MOTOR11, MOTOR12, TACH11, TACH12};
        else
            motorPins = {MOTOR12, MOTOR11, TACH12, TACH11};
        break;
    case M2:
        if (!_motorFlip)
            motorPins = {MOTOR21, MOTOR22, TACH21, TACH22};
        else
            motorPins = {MOTOR22, MOTOR21, TACH22, TACH21};
        break;
    case M3:
        if (!_motorFlip)
            motorPins = {MOTOR31, MOTOR32, TACH31, TACH32};
        else
            motorPins = {MOTOR32, MOTOR31, TACH32, TACH31};
        break;
    case M4:
        if (!_motorFlip)
            motorPins = {MOTOR41, MOTOR42, TACH41, TACH42};
        else
            motorPins = {MOTOR42, MOTOR41, TACH42, TACH41};
        break;
    }
}
void EV3Motor::begin()
{
    EVO::getInstance().selectI2CChannel(SX1509_CHANNEL);
    if (!SX1509Initialized)
    {
        if (io.begin(SX1509_ADDR) == false)
        {
            Serial.println("Failed to communicate. Check wiring and address of SX1509.");
        }
        else
        {
            SX1509Initialized = true;
        }
    }
    io.pinMode(motorPins.power1, ANALOG_OUTPUT);
    io.pinMode(motorPins.power2, ANALOG_OUTPUT);
    encoder.attachFullQuad(motorPins.tach1, motorPins.tach2);
    encoder.clearCount();
}

// Method to get current angle
int EV3Motor::getAngle()
{
    return encoder.getCount();
}

// Method to reset angle
void EV3Motor::resetAngle()
{
    encoder.setCount(0);
}

// Method to stop the motor
void EV3Motor::coast()
{
    this->run(0);
}

// Method to brake the motor
void EV3Motor::brake()
{
    EVO::getInstance().selectI2CChannel(SX1509_CHANNEL);
    io.analogWrite(motorPins.power1, 255);
    io.analogWrite(motorPins.power2, 255);
}

// Method to run the motor at a specified speed
void EV3Motor::run(int speed)
{
    speed = clamp(speed, -255, 255);
    EVO::getInstance().selectI2CChannel(SX1509_CHANNEL);
    if (speed > 0)
    {
        io.analogWrite(motorPins.power1, speed);
        io.analogWrite(motorPins.power2, 0);
    }
    else if (speed < 0)
    {
        io.analogWrite(motorPins.power1, 0);
        io.analogWrite(motorPins.power2, -speed);
    }
    else
    {
        io.analogWrite(motorPins.power1, 0);
        io.analogWrite(motorPins.power2, 0);
    }
}

// Method to run the motor for a specified number of degrees
void EV3Motor::runDegrees(int speed, int degrees)
{
    this->resetAngle();
    while (abs(this->getAngle()) < degrees)
    {
        // Serial.println(this->getAngle());
        this->run(speed);
    }
    this->brake();
}