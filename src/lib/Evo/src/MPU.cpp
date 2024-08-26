#include <MPU.h>

MPU::MPU()
{

}

  
  
void MPU::begin()
{
    _yaw = 0;
    Wire1.begin(17, 18);
    MPU9250Setting setting;
    setting.accel_fs_sel = ACCEL_FS_SEL::A16G;
    setting.gyro_fs_sel = GYRO_FS_SEL::G2000DPS;
    setting.mag_output_bits = MAG_OUTPUT_BITS::M16BITS;
    setting.fifo_sample_rate = FIFO_SAMPLE_RATE::SMPL_125HZ;
    setting.gyro_fchoice = 0x03;
    setting.gyro_dlpf_cfg = GYRO_DLPF_CFG::DLPF_20HZ;
    setting.accel_fchoice = 0x01;
    setting.accel_dlpf_cfg = ACCEL_DLPF_CFG::DLPF_21HZ;
    if (!mpu.setup(0x68, MPU9250Setting(), Wire1)) 
    {
        Serial.println(F("Failed to boot MPU9250"));
    }

    mpu.setAccBias(54.66, 30.68, -9.59);
    mpu.setGyroBias(-3.24, -0.52, 0.05);
    mpu.setMagBias(178.63, 319.09, 30.57);
    mpu.setMagScale(0.80, 0.68, 3.68);
    mpu.setMagneticDeclination(0.03);
    
}


float MPU::getHeading(int offset)
{
    if (mpu.update() && (millis() - _lastms) > 50)
    {
        _lastms = millis();
        _yaw = mpu.getYaw();
        
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
