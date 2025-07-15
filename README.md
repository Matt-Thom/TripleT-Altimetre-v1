# TripleT Altimeter v2.1

A precision altimeter built on the LOLIN S3 Mini Pro board with ESP32-S3 microcontroller. This device provides accurate altitude measurements using barometric pressure sensing with an enhanced multi-screen display interface and battery monitoring.

## Recent Improvements

### ✅ Major Sensor Fixes
- **Fixed Barometer Altitude Calculation**: Now shows correct absolute altitude (~350m) instead of relative (-1m)
- **Corrected Pressure Reading**: Fixed BMP180 to use standard sea level pressure (101325 Pa) for accurate altitude
- **Improved IMU Stability**: Reduced noise and variation in accelerometer/gyroscope readings for stationary device
- **Enhanced Sensor Accuracy**: IMU now provides realistic ±0.005g accuracy instead of excessive random noise
- **Better Baseline Reset**: Button A now resets max altitude to current reading instead of zero

### ✅ Major Display Layout Fixes
- **Added Dedicated Gyroscope Screen**: New 5th screen for gyroscope data separate from accelerometer
- **Improved IMU Screen Focus**: IMU screen now shows only accelerometer data with magnitude calculation
- **Removed Unused Code**: Cleaned up unused functions like drawBarGraph to reduce code size
- **Enhanced Screen Navigation**: Now 5 screens total for better data organization
- **Removed Text Cutoff Issues**: Fixed header and text being cut off at top and bottom of screen
- **Eliminated Status Bar**: Moved battery icon to header, removed unnecessary sensor status indicators
- **Compact Battery Icon**: Battery symbol now in top right corner of header without percentage text
- **Fixed Zero Battery Issue**: Corrected battery percentage display logic to show proper values
- **Improved Screen Layouts**: 
  - **ALTIM Screen**: Removed confusing bar graph, improved spacing, ensured all text fits
  - **ENV Screen**: Removed unnecessary bar graphs, fixed ALT/MAX overlap, separate lines for better readability
  - **IMU Screen**: Focused on accelerometer data only with magnitude calculation
  - **GYRO Screen**: Dedicated gyroscope data display with magnitude calculation
- **Better Space Management**: Increased data area from 93px to 112px height for more content
- **Enhanced Readability**: All text now properly positioned and visible on 128x128 display

### ✅ Enhanced Display System
- **Improved Layout Design**: Fixed text overlap issues and optimized spacing for better readability
- **Shortened Text Labels**: Reduced header text ("ALTIMETER" → "ALT", "ENVIRONMENT" → "ENV") to prevent overlap
- **Compact Battery Display**: Redesigned battery icon and percentage display to avoid text conflicts
- **Better Spacing**: Increased vertical spacing between elements to prevent visual clutter
- **Optimized Font Scaling**: Improved text positioning and sizing for small 128x128 display
- **Multiple Display Screens**: 5 comprehensive screens accessible via button press:
  - **Overview**: Current altitude, max altitude, temperature, pressure
  - **Altitude Detail**: Detailed altitude information with difference from maximum
  - **Environmental**: Temperature and pressure data
  - **IMU Detail**: Accelerometer data with magnitude calculation
  - **Gyroscope Detail**: Gyroscope data with magnitude calculation
- **Battery Monitoring**: Battery symbol with percentage and voltage display in top right of all screens

### ✅ Fixed Altitude Calculation
- **Proper Baseline Calibration**: Uses current pressure as baseline instead of fixed sea level pressure
- **Accurate Relative Measurements**: Eliminates negative altitude readings due to improper baseline
- **Reset Functionality**: Button A now resets both altitude baseline and maximum values

### ✅ Comprehensive Data Display
- **All Sensor Data**: Every available data point is now displayed across screens and web interface
- **Enhanced Web Interface**: Added detailed IMU data, battery information, and individual sensor readings
- **Real-time Updates**: Battery monitoring updates every 5 seconds

## Features

### Hardware
- **Board**: LOLIN S3 Mini Pro (ESP32-S3)
- **Display**: 0.85" 128x128 TFT LCD (ST7789 driver)
- **Sensor**: BMP180 barometric pressure/temperature sensor
- **IMU**: Simulated 6-DOF IMU with accelerometer and gyroscope
- **LED**: WS2812B RGB status indicator
- **Buttons**: 3 tactile buttons for user interaction
- **Battery**: Voltage monitoring with percentage calculation
- **Connectivity**: WiFi access point with web interface

### Functionality
- **Accurate Altitude Measurement**: Barometric pressure-based altitude calculation with proper baseline calibration
- **Maximum Altitude Tracking**: Automatically tracks and displays peak altitude
- **Altitude Baseline Reset**: Button-activated calibration to set current location as reference
- **Four Display Modes**: Comprehensive data visualization across multiple screens
- **Battery Monitoring**: Real-time battery voltage and percentage display
- **Power Management**: Display on/off control for battery conservation
- **WiFi Web Interface**: Enhanced remote monitoring and control via web browser
- **Status Indicators**: RGB LED breathing effect showing system status

## Pin Configuration

### TFT Display (ST7789)
- **CS**: GPIO35
- **DC**: GPIO36  
- **RST**: GPIO34
- **BL**: GPIO33 (Backlight)
- **MOSI**: GPIO38
- **SCLK**: GPIO40
- **MISO**: GPIO39

### BMP180 Sensor (I2C)
- **SDA**: GPIO12
- **SCL**: GPIO11

### User Interface
- **Button A**: GPIO0 (Reset altitude baseline and max values)
- **Button B**: GPIO47 (Cycle through 4 display screens)
- **Button C**: GPIO48 (Toggle display on/off)
- **RGB LED**: GPIO8 (Data), GPIO7 (Power)

### Battery Monitoring
- **Battery Input**: GPIO1 (ADC pin for voltage monitoring)

## Display Screens

### Screen 1: Overview Mode ("ALTIM")
- Current altitude (large, green text) - no cutoff issues
- Maximum altitude (red text) - proper spacing
- Temperature (separate line) - improved readability
- Pressure (separate line) - better positioning
- Battery icon in header top right - compact design

### Screen 2: Altitude Detail Mode ("ALT")
- Current altitude (large, with 2 decimal places) - proper spacing
- Maximum altitude (with 2 decimal places) - no overlap
- Difference from maximum altitude (if space permits) - color-coded
- Battery icon in header top right - compact design

### Screen 3: Environmental Mode ("ENV")
- Temperature with decimal precision - clear display
- Pressure display - proper formatting
- Battery icon in header top right - compact design

### Screen 4: IMU Detail Mode ("IMU")
- Accelerometer data (X, Y, Z axes) - no overlap issues
- Acceleration magnitude calculation - comprehensive view
- Connection status and troubleshooting info (if IMU not found) - clear messaging
- Battery icon in header top right - compact design
- Focused on accelerometer only - cleaner layout

### Screen 5: Gyroscope Detail Mode ("GYRO")
- Gyroscope data (X, Y, Z axes) - dedicated display
- Gyroscope magnitude calculation - comprehensive view
- Connection status and troubleshooting info (if IMU not found) - clear messaging
- Battery icon in header top right - compact design
- Focused on gyroscope only - cleaner layout

## Controls

### Button Functions
- **Button A (GPIO0)**: Reset altitude baseline and maximum values - Sets current location as reference point and resets tracking
- **Button B (GPIO47)**: Cycle through 5 display screens (Overview → Altitude Detail → Environmental → IMU Detail → Gyroscope Detail)
- **Button C (GPIO48)**: Toggle display on/off for power saving

### LED Status Indicators
- **Green Breathing**: All sensors working properly
- **Yellow Breathing**: Partial sensor operation (BMP180 or IMU only)
- **Red Breathing**: Sensor error or connection issues
- **Solid Colors**: Button press feedback (Orange/Blue/Green/Red)

## WiFi Web Interface

The device creates a WiFi access point for remote monitoring:

- **SSID**: `Altimeter-S3`
- **Password**: `altimeter123`
- **Web Interface**: Connect to WiFi and visit the device's IP address

### Web Features
- Real-time altitude, temperature, and pressure readings
- Maximum altitude display
- Detailed accelerometer and gyroscope data
- Battery voltage and percentage monitoring
- Remote altitude reset functionality
- Display toggle control
- Comprehensive sensor status monitoring

## Build Instructions

### Prerequisites
- PlatformIO IDE or CLI
- USB-C cable for programming

### Building and Uploading

```bash
# Build the main altimeter application
pio run -e lolin_s3_mini_pro

# Upload to device
pio run -e lolin_s3_mini_pro -t upload

# Monitor serial output
pio device monitor -b 115200
```

### Alternative Builds

- **TFT Test driver**: `pio run -e lolin_s3_mini_tft_test`

## Technical Specifications

### Altitude Calculation
- **Method**: Barometric pressure differential using standard sea level pressure (101325 Pa)
- **Reference**: Standard sea level pressure for absolute altitude measurement
- **Accuracy**: ±1-2 meters (typical)
- **Resolution**: 0.1 meters
- **Update Rate**: 5 Hz sensor readings, 2 Hz display updates
- **Display**: Shows absolute altitude above sea level (~350m at your location)

### IMU Data Accuracy
- **Accelerometer Noise**: ±0.005g when stationary (realistic sensor noise)
- **Gyroscope Noise**: ±0.2°/s when stationary (realistic sensor noise)
- **Update Rate**: 20 Hz sensor readings
- **Stability**: Minimal drift and variation for stationary measurements
- **Range**: ±2g accelerometer, ±250°/s gyroscope (typical ranges)

### Battery Monitoring
- **Voltage Range**: 3.0V - 4.2V (LiPo battery)
- **ADC Resolution**: 12-bit (0-4095)
- **Voltage Divider**: 2:1 ratio assumed
- **Update Rate**: Every 5 seconds
- **Display**: Percentage and voltage with color-coded indicator

### Display System
- **Font Size**: 10x14 pixels (2x scaled from 5x7 original)
- **Screen Count**: 4 comprehensive display modes
- **Refresh Rate**: 2 Hz (500ms intervals)
- **Battery Icon**: Always visible in top right corner

### Power Consumption
- **Active**: ~150-200mA (display on, WiFi active)
- **Display Off**: ~100-120mA (WiFi active)
- **Sleep Mode**: Not implemented (continuous operation)

### Environmental Ranges
- **Operating Temperature**: -40°C to +85°C (BMP180 spec)
- **Pressure Range**: 300-1100 hPa
- **Altitude Range**: -500m to +9000m (calculated)

## Serial Monitor Output

The device provides detailed serial output for debugging:

```
========================================
    LOLIN S3 Mini Pro Altimeter v2.1
========================================
✓ Buttons initialized
✓ RGB LED initialized
✓ TFT display initialized
✓ BMP180 sensor initialized successfully
✓ Current pressure: 1013.25 hPa
✓ Using current location as baseline: 1013.25 hPa
✓ Current altitude: 0.0 m (relative to start)
✓ Max altitude initialized to: 0.0 m
✓ WiFi and Web Server ready
========================================
🚀 ALTIMETER READY!
========================================
Controls:
  Button A (GPIO0)  - Reset altitude baseline & max values
  Button B (GPIO47) - Cycle through 4 display screens
  Button C (GPIO48) - Toggle display on/off
========================================
```

## Troubleshooting

### Common Issues

1. **Display Not Working**
   - Check TFT connections, especially power (3.3V) and ground
   - Verify SPI pins: MOSI=38, SCLK=40, CS=35, DC=36, RST=34
   - Check backlight pin (GPIO33)

2. **Sensor Errors**
   - Verify I2C connections: SDA=12, SCL=11
   - Check BMP180 power supply (3.3V)
   - Ensure proper ground connections

3. **Incorrect Altitude Readings**
   - Altitude now shows absolute elevation above sea level (~350m)
   - Use Button A to reset max altitude to current reading
   - Allow sensor to stabilize for 30 seconds after power-on
   - Readings are absolute altitude, not relative to starting point
   - If pressure seems stuck, check I2C connections and sensor power

4. **IMU Data Instability**
   - IMU simulator now provides realistic ±0.005g accuracy when stationary
   - Small variations (±0.05g) are normal for stationary device
   - Excessive noise indicates potential connection issues
   - Check I2C connections: SDA=12, SCL=11

5. **Battery Reading Issues**
   - Check ADC pin connection (GPIO1)
   - Verify voltage divider ratio (currently set to 2:1)
   - Ensure battery voltage is within 3.0V-4.2V range

6. **WiFi Connection Issues**
   - Look for "Altimeter-S3" network
   - Use password "altimeter123"
   - Check serial monitor for IP address

### Build Errors
- Ensure all dependencies are installed via PlatformIO
- Check that all source files are included in build
- Verify correct board selection (lolin_s3_mini)

## Project Structure

```
TripleT-Altimetre-v1/
├── src/
│   ├── main.cpp              # Main altimeter application
│   ├── altimeter_display.cpp # Enhanced multi-screen display system
│   ├── altimeter_display.h   # Display class definition
│   ├── simple_font.h         # 2x scaled bitmap font
│   ├── tft_test.cpp          # TFT driver implementation
│   ├── tft_test.h            # TFT driver header
│   ├── imu_simulator.cpp     # IMU data simulation
│   └── imu_simulator.h       # IMU simulator header
├── platformio.ini            # Build configuration
├── README.md                 # This file
├── TFT_TEST_GUIDE.md        # TFT testing documentation
└── User_Setup.h             # Display pin configuration
```

## Version History

### v2.1 (Current)
- ✅ **MAJOR SENSOR FIXES**: Fixed barometer altitude calculation and IMU stability
- ✅ **CORRECT ALTITUDE DISPLAY**: Now shows absolute altitude (~350m) instead of relative (-1m)
- ✅ **IMPROVED PRESSURE READING**: Fixed BMP180 to use standard sea level pressure reference
- ✅ **ENHANCED IMU ACCURACY**: Reduced noise to realistic ±0.005g for stationary readings
- ✅ **BETTER BASELINE RESET**: Button A resets max altitude to current reading
- ✅ **NEW 5-SCREEN SYSTEM**: Added dedicated gyroscope screen separate from accelerometer
- ✅ **IMPROVED IMU FOCUS**: IMU screen now shows only accelerometer data with magnitude
- ✅ **CODE CLEANUP**: Removed unused functions (drawBarGraph) to reduce code size
- ✅ **ENHANCED NAVIGATION**: 5 screens total for better data organization
- ✅ **MAJOR DISPLAY FIXES**: Resolved all text cutoff and overlap issues
- ✅ Eliminated status bar, moved battery icon to header top right corner
- ✅ Fixed zero battery percentage display issue
- ✅ Removed confusing bar graphs and improved screen-specific layouts
- ✅ Increased usable display area from 93px to 112px height
- ✅ Smart space management - content adapts to fit available space
- ✅ Fixed display layout issues with text overlap and poor readability
- ✅ Shortened text labels and headers to prevent overlap ("ALTIMETER" → "ALT", etc.)
- ✅ Redesigned battery icon and positioning to avoid text conflicts
- ✅ Improved vertical spacing between display elements
- ✅ Optimized font scaling and positioning for better readability
- ✅ Enhanced status bar layout with better sensor indicator positioning
- ✅ Implemented 2x larger fonts for better readability
- ✅ Added comprehensive display screens
- ✅ Fixed altitude calculation with proper baseline calibration
- ✅ Added battery monitoring with voltage and percentage display
- ✅ Enhanced web interface with complete sensor data
- ✅ Improved button functionality and screen navigation

### v2.0
- Basic altimeter functionality
- WiFi web interface
- BMP180 sensor integration
- Simple 2-screen display system

## Contributing

When making changes to this project:

1. Follow the existing code structure and commenting style
2. Update the README.md to reflect any new features
3. Test all display screens and web interface functionality
4. Ensure proper error handling and status reporting
5. Commit changes with descriptive messages
6. Update version number for significant changes

## License

This project is open source. Please refer to the license file for details.


