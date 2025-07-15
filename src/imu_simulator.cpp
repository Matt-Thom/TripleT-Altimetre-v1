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
    
    // Generate realistic IMU data with minimal noise for stationary device
    float time_sec = now / 1000.0;
    
    // Simulate very small movements that occur in real stationary devices
    // Much smaller amplitudes and slower frequencies for realistic behavior
    
    // Simulate tiny lateral movements (X-axis) - typical sensor noise
    accel_x = 0.05 * sin(time_sec * 0.2) +     // Very slow drift
              0.02 * sin(time_sec * 1.1) +     // Small vibrations
              (random(-5, 5) / 1000.0);        // Minimal noise ±0.005g
    
    // Simulate tiny forward/back movements (Y-axis)
    accel_y = 0.04 * cos(time_sec * 0.15) +    // Very slow drift
              0.015 * cos(time_sec * 0.9) +    // Small vibrations
              (random(-5, 5) / 1000.0);        // Minimal noise ±0.005g
    
    // Simulate vertical movements (Z-axis) - gravity plus tiny variations
    float vertical_variation = 0.03 * sin(time_sec * 0.3) + 
                              0.01 * sin(time_sec * 2.1);  // Very small variations
    accel_z = 1.0 + vertical_variation + (random(-3, 3) / 1000.0);  // 1g baseline + tiny variations
    
    // Simulate very small rotational movements (much more stable)
    gyro_x = 0.5 * sin(time_sec * 0.1) + (random(-2, 2) / 10.0);   // Tiny pitch movements
    gyro_y = 0.3 * cos(time_sec * 0.12) + (random(-2, 2) / 10.0);  // Tiny roll movements
    gyro_z = 0.2 * sin(time_sec * 0.08) + (random(-1, 1) / 10.0);  // Tiny yaw movements
    
    return true;
} 