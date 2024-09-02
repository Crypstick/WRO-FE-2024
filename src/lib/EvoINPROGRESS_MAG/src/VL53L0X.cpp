#include <VL53L0X.h>

VL53L0X::VL53L0X(uint8_t i2cPort)
{
    _i2cPort = i2cPort;
}

void VL53L0X::begin()
{
    Serial.println("begining");
    EVO::getInstance().selectI2CChannel(this->_i2cPort);
    if (!this->lox.begin())
    {
        Serial.println(F("Failed to boot VL53L0X"));
    }
}

int VL53L0X::getDistance()
{
    EVO::getInstance().selectI2CChannel(this->_i2cPort);
    if (millis() - this->_lastms > 20){
        lox.rangingTest(&this->measure, false); // pass in 'true' to get debug data printout!

        if (this->measure.RangeStatus != 4)
        {
            this->_reading = this->measure.RangeMilliMeter;
            
        }
        else
        {
            this->_reading = this->measure.RangeMilliMeter;
        }
        return this->_reading;
        this->_lastms = millis();
    }
    return _reading;
}