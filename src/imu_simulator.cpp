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
    
    // Always succeed for testing purposes
    // In real implementation, this would attempt to communicate with actual IMU
    initialized = true;  // Force success for testing
    
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
    
    // Simulate more dynamic movement patterns
    // Include occasional higher accelerations that would occur during actual use
    float movement_intensity = 1.0 + 0.5 * sin(time_sec * 0.1);  // Vary movement intensity over time
    
    // Simulate lateral movements (X-axis) - includes occasional higher accelerations
    accel_x = 0.3 * movement_intensity * sin(time_sec * 0.8) + 
              0.1 * sin(time_sec * 2.1) +  // Higher frequency component
              (random(-20, 20) / 1000.0);   // Noise
    
    // Simulate forward/back movements (Y-axis) 
    accel_y = 0.25 * movement_intensity * cos(time_sec * 0.6) + 
              0.15 * cos(time_sec * 1.8) +  // Higher frequency component
              (random(-20, 20) / 1000.0);   // Noise
    
    // Simulate vertical movements (Z-axis) - gravity plus movement
    float vertical_movement = 0.4 * movement_intensity * sin(time_sec * 0.4) + 
                             0.2 * sin(time_sec * 3.2);  // Include higher frequency vibrations
    accel_z = 1.0 + vertical_movement + (random(-25, 25) / 1000.0);  // 1g baseline + movement + noise
    
    // Simulate small rotational movements
    gyro_x = 2.0 * sin(time_sec * 0.3) + (random(-20, 20) / 10.0);  // Small pitch movements
    gyro_y = 1.5 * cos(time_sec * 0.4) + (random(-20, 20) / 10.0);  // Small roll movements
    gyro_z = 0.5 * sin(time_sec * 0.6) + (random(-10, 10) / 10.0);  // Small yaw movements
    
    return true;
} 