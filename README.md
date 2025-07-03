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
- [x] Display integration (TFT screen)
- [x] Maximum altitude tracking
- [x] Data logging (to SPIFFS)
- [x] Battery monitoring (Voltage display)
- [x] User interface (Button for Max Alt Reset)

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
- TFT_eSPI library by Bodmer (for display)
- SPIFFS (for data logging)

## Suggestions for Future Enhancements
- **Advanced User Interface:**
    - Use more buttons (if available on the triple base) or touch screen capabilities (if display supports it) for navigation.
    - Menu system for settings (e.g., sea level pressure calibration, logging interval).
    - Start/Stop data logging via UI.
    - Option to clear SPIFFS data log.
- **Improved Data Logging:**
    - Real-time clock (RTC) module for accurate timestamps instead of `millis()`.
    - Option to log data to an SD card if a module is added (larger storage, easier retrieval).
    - Data format selection (e.g., JSON, binary).
- **Power Management:**
    - Implement deep sleep modes to conserve battery when not actively logging or if altitude is stable.
    - More accurate battery percentage calculation (e.g., using a discharge curve for the specific LiPo battery).
- **Sensor Calibration:**
    - Allow manual input of current sea level pressure for more accurate altitude readings.
    - Temperature compensation for sensor readings if not already handled adequately by the library.
- **Connectivity & Data Offloading:**
    - Use Wi-Fi or Bluetooth to transmit data to a phone or computer.
    - Web server on ESP32 to view live data or download logs.
- **Physical Enhancements:**
    - 3D printed case for the altimeter.
    - Buzzer for audible alerts (e.g., apogee reached, low battery).
- **Code Refinements:**
    - ADC calibration for more accurate battery voltage.
    - More robust error handling and display messages.
    - Modularize code into separate files for better organization (e.g., display_handler.cpp, logger.cpp).


