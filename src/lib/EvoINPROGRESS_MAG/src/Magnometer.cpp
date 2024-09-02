#include <Magnometer.h>

Magnometer::Magnometer(uint8_t i2cPort, float declinationAngle)
{
    _i2cPort = i2cPort;
    _declinationAngle = declinationAngle;
}
void Magnometer::begin()
{
    EVO::getInstance().selectI2CChannel(this->_i2cPort);
    if (!magnometer.begin())
    {
        Serial.println("Ooops, no HMC5883 detected ... Check your wiring!");
    }
}
float Magnometer::getHeading(float offsetHeading)
{
    EVO::getInstance().selectI2CChannel(this->_i2cPort);
    sensors_event_t event;
    magnometer.getEvent(&event);
    float heading = atan2(event.magnetic.y, event.magnetic.x);
    heading += this->_declinationAngle + (offsetHeading / 180 * PI);
    while (heading < -PI or heading > PI)
    {
        if (heading < -PI)
            heading += 2 * PI;
        if (heading > PI)
            heading -= 2 * PI;
    }

    float headingDegrees = heading * 180 / M_PI;
    return headingDegrees;
}
void Magnometer::setTargetHeading(float target)
{
    this->_targetHeading = target;
}
float Magnometer::getTargetHeading()
{
    return this->_targetHeading;
}

float Magnometer::getRelativeHeading()
{
    return this->getHeading(-_targetHeading);
}