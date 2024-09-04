#include <MPU.h>

MPU::MPU()
{

}

  
  
void MPU::begin()
{
    Wire1.begin(17, 18);
    _yaw = 0;
    int status = mpu.begin();

    float hxb = 17.168816; // mag bias of 10 uT
    float hxs = 0.786952; // mag scale factor of 0.97
    mpu.setMagCalX(hxb,hxs);

    float hyb = 28.570538; // mag bias of 10 uT
    float hys = 0.797671; // mag scale factor of 0.97
    mpu.setMagCalY(hyb,hys);

    float hzb = -15.896074; // mag bias of 10 uT
    float hzs = 2.102503; // mag scale factor of 0.97
    mpu.setMagCalZ(hzb,hzs);
    if (status < 0) {
        Serial.println("IMU initialization unsuccessful");
        Serial.println("Check IMU wiring or try cycling power");
        Serial.print("Status: ");
        Serial.println(status);
        while(1) {}
    }
    // setting the accelerometer full scale range to +/-8G 
    mpu.setAccelRange(MPU9250::ACCEL_RANGE_8G);
    // setting the gyroscope full scale range to +/-500 deg/s
    mpu.setGyroRange(MPU9250::GYRO_RANGE_500DPS);
    // setting DLPF bandwidth to 20 Hz
    mpu.setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_20HZ);
    // setting SRD to 19 for a 50 Hz update rate
    mpu.setSrd(19);
}


float MPU::getHeading(int offset)
{
    if ((millis() - _lastms) > 50)
    {
        _lastms = millis();
        mpu.readSensor();
        _yaw = std::atan2(mpu.getMagY_uT(), mpu.getMagX_uT()) * 180 / PI;
        
        _yaw -= offset;
        while (_yaw < 0 or _yaw > 360)
        {
            if (_yaw < 0)
                _yaw += 360;
            if (_yaw > 360)
                _yaw -= 360;
        }
    }
    return _yaw;
}

void MPU::setTargetHeading(int target)
{
    this->_targetHeading = target;
}
int MPU::getTargetHeading()
{
    return this->_targetHeading;
}

float MPU::getRelativeHeading()
{
    return this->getHeading(-_targetHeading);
}
