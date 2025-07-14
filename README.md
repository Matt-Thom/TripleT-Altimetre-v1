# TripleT Altimeter v2.1

A precision altimeter built on the LOLIN S3 Mini Pro board with ESP32-S3 microcontroller. This device provides accurate altitude measurements using barometric pressure sensing with an enhanced multi-screen display interface and battery monitoring.

## Recent Improvements

### ✅ Enhanced Display System
- **Improved Layout Design**: Fixed text overlap issues and optimized spacing for better readability
- **Shortened Text Labels**: Reduced header text ("ALTIMETER" → "ALTIM", "ENVIRONMENT" → "ENV") to prevent overlap
- **Compact Battery Display**: Redesigned battery icon and percentage display to avoid text conflicts
- **Better Spacing**: Increased vertical spacing between elements to prevent visual clutter
- **Optimized Font Scaling**: Improved text positioning and sizing for small 128x128 display
- **Multiple Display Screens**: 4 comprehensive screens accessible via button press:
  - **Overview**: Current altitude, max altitude, temperature, pressure, and acceleration
  - **Altitude Detail**: Detailed altitude information with difference from maximum
  - **Environmental**: Temperature and pressure with bar graph visualizations
  - **IMU Detail**: Full accelerometer and gyroscope data display
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
- Current altitude (large, green text) - improved positioning
- Maximum altitude (red text) - better spacing
- Temperature and pressure (compact display) - shortened labels
- Altitude bar graph visualization - optimized width
- Acceleration magnitude (if IMU available) - better alignment
- Battery symbol in top right corner - compact design

### Screen 2: Altitude Detail Mode ("ALT")
- Current altitude (large, with 2 decimal places) - improved spacing
- Maximum altitude (with 2 decimal places) - better layout
- Difference from maximum altitude (color-coded) - enhanced positioning
- Battery symbol in top right corner - optimized placement

### Screen 3: Environmental Mode ("ENV")
- Current and max altitude (compact, at top) - improved alignment
- Temperature with bar graph visualization - better spacing
- Pressure with bar graph visualization - optimized layout
- Battery symbol in top right corner - compact design

### Screen 4: IMU Detail Mode ("IMU")
- Current and max altitude (compact, at top) - improved positioning
- Detailed accelerometer data (X, Y, Z axes) - better spacing
- Detailed gyroscope data (X, Y, Z axes) - optimized layout
- Connection status and troubleshooting info - enhanced readability
- Battery symbol in top right corner - compact design

## Controls

### Button Functions
- **Button A (GPIO0)**: Reset altitude baseline and maximum values - Sets current location as reference point and resets tracking
- **Button B (GPIO47)**: Cycle through 4 display screens (Overview → Altitude Detail → Environmental → IMU Detail)
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
- **Method**: Barometric pressure differential with proper baseline calibration
- **Reference**: Current pressure reading as baseline (not fixed sea level)
- **Accuracy**: ±1-2 meters (typical)
- **Resolution**: 0.1 meters
- **Update Rate**: 5 Hz sensor readings, 2 Hz display updates

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
   - Use Button A to reset altitude baseline at known location
   - Allow sensor to stabilize for 30 seconds after power-on
   - Readings are now relative to starting point, not absolute altitude

4. **Battery Reading Issues**
   - Check ADC pin connection (GPIO1)
   - Verify voltage divider ratio (currently set to 2:1)
   - Ensure battery voltage is within 3.0V-4.2V range

5. **WiFi Connection Issues**
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
- ✅ Fixed display layout issues with text overlap and poor readability
- ✅ Shortened text labels and headers to prevent overlap ("ALTIMETER" → "ALTIM", etc.)
- ✅ Redesigned battery icon and positioning to avoid text conflicts
- ✅ Improved vertical spacing between display elements
- ✅ Optimized font scaling and positioning for better readability
- ✅ Enhanced status bar layout with better sensor indicator positioning
- ✅ Implemented 2x larger fonts for better readability
- ✅ Added 4 comprehensive display screens
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


