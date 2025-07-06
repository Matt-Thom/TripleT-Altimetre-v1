#include "imu_simulator.h"
#include <math.h>

IMUSimulator::IMUSimulator() {
    accel_x = 0.0;
    accel_y = 0.0;
    accel_z = 1.0;  // 1g downward when stationary
    gyro_x = 0.0;
    gyro_y = 0.0;
    gyro_z = 0.0;
    last_update = 0;
    initialized = false;
}

bool IMUSimulator::begin() {
    // Simulate IMU initialization
    delay(100);
    
    // Simulate 80% chance of successful initialization
    // In real implementation, this would attempt to communicate with actual IMU
    initialized = (random(100) < 80);
    
    if (initialized) {
        Serial.println("IMU Simulator: Initialized successfully");
        last_update = millis();
    } else {
        Serial.println("IMU Simulator: Initialization failed (simulated)");
    }
    
    return initialized;
}

bool IMUSimulator::update() {
    if (!initialized) {
        return false;
    }
    
    unsigned long now = millis();
    if (now - last_update < 50) {  // Update at 20Hz
        return true;
    }
    
    last_update = now;
    
    // Generate realistic IMU data with some noise and movement simulation
    float time_sec = now / 1000.0;
    
    // Simulate some gentle movement and vibration
    accel_x = 0.05 * sin(time_sec * 0.5) + (random(-10, 10) / 1000.0);  // Small lateral movements
    accel_y = 0.03 * cos(time_sec * 0.7) + (random(-10, 10) / 1000.0);  // Small forward/back movements
    accel_z = 1.0 + 0.02 * sin(time_sec * 1.2) + (random(-15, 15) / 1000.0);  // Mostly 1g with small variations
    
    // Simulate small rotational movements
    gyro_x = 2.0 * sin(time_sec * 0.3) + (random(-20, 20) / 10.0);  // Small pitch movements
    gyro_y = 1.5 * cos(time_sec * 0.4) + (random(-20, 20) / 10.0);  // Small roll movements
    gyro_z = 0.5 * sin(time_sec * 0.6) + (random(-10, 10) / 10.0);  // Small yaw movements
    
    return true;
} 