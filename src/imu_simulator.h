#ifndef IMU_SIMULATOR_H
#define IMU_SIMULATOR_H

#include <Arduino.h>

class IMUSimulator {
private:
    float accel_x, accel_y, accel_z;
    float gyro_x, gyro_y, gyro_z;
    unsigned long last_update;
    bool initialized;
    
public:
    IMUSimulator();
    bool begin();
    bool update();
    
    float getAccelX() { return accel_x; }
    float getAccelY() { return accel_y; }
    float getAccelZ() { return accel_z; }
    float getGyroX() { return gyro_x; }
    float getGyroY() { return gyro_y; }
    float getGyroZ() { return gyro_z; }
    
    bool isAvailable() { return initialized; }
};

#endif // IMU_SIMULATOR_H 