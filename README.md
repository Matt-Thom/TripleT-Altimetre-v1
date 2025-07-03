# TripleT Altimeter v1

This is a very basic altimeter for model rocketry.
Using a barometric pressure sensor it will calculate altitude and display it in the screen.
It will store the largest altitude recorded until reset.

## Hardware
- D1 Mini compatible (S3 Mini Pro V1.0.0 - LOLIN WIFI Bluetooth IOT Board based ESP32-S3FH4R2 TFT IMU MicroPython Arduino Compatible)
- D1 mini battery hat (for power) 
- BMP180 Shield
- D1 mini Pro Triple Base

## Software Components

### Libraries
- **BMP180 Library**: `enjoyneering/BMP180@^1.0.0`
  - Comprehensive library for Bosch BMP180 barometric pressure & temperature sensor
  - Supports I2C communication
  - Features: Temperature reading, pressure reading, altitude calculation, sea level pressure calculation
  - Compatible with ESP32 and Arduino frameworks

### Features
- Temperature measurement in Celsius
- Barometric pressure measurement in Pascals
- Altitude calculation in meters
- Serial output for debugging and monitoring
- 2-second reading interval

## BMP180 Sensor Specifications
- Supply voltage: 1.8V - 3.6V
- Pressure range: 30,000Pa to 110,000Pa
- Operating temperature: 0°C to +65°C
- Typical resolution: 1Pa / 0.1°C
- Typical accuracy: ±100Pa / ±1.0°C
- Communication: I2C interface

## Wiring
The BMP180 sensor should be connected via I2C:
- VCC: 3.3V
- GND: Ground
- SDA: I2C Data line (GPIO 21 on ESP32-S3)
- SCL: I2C Clock line (GPIO 22 on ESP32-S3)

## Usage
1. Connect the BMP180 sensor to the ESP32-S3 board
2. Upload the code using PlatformIO
3. Open the serial monitor at 115200 baud
4. The device will display temperature, pressure, and altitude readings every 2 seconds

## Development Status
- [x] Basic project structure setup
- [x] BMP180 library integration
- [x] Basic sensor reading functionality
- [ ] TFT Display integration
- [ ] Maximum altitude tracking
- [ ] Data logging
- [ ] Battery monitoring
- [ ] User interface

## Build Instructions
1. Install PlatformIO
2. Clone this repository
3. Open the project in PlatformIO
4. Build and upload to your ESP32-S3 board

## Dependencies
- PlatformIO
- ESP32 Arduino Framework
- Wire library (I2C communication)
- BMP180 library by enjoyneering


