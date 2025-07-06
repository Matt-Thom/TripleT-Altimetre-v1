# TripleT Altimeter v2.1

A precision altimeter built on the LOLIN S3 Mini Pro board with ESP32-S3 microcontroller. This device provides accurate altitude measurements using barometric pressure sensing with a clean, focused display interface.

## Features

### Hardware
- **Board**: LOLIN S3 Mini Pro (ESP32-S3)
- **Display**: 0.85" 128x128 TFT LCD (ST7789 driver)
- **Sensor**: BMP180 barometric pressure/temperature sensor
- **LED**: WS2812B RGB status indicator
- **Buttons**: 3 tactile buttons for user interaction
- **Connectivity**: WiFi access point with web interface

### Functionality
- **Accurate Altitude Measurement**: Barometric pressure-based altitude calculation
- **Maximum Altitude Tracking**: Automatically tracks and displays peak altitude
- **Altitude Zeroing**: Button-activated calibration to set current location as reference
- **Dual Display Modes**: Main overview and detailed altitude view
- **Power Management**: Display on/off control for battery conservation
- **WiFi Web Interface**: Remote monitoring and control via web browser
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
- **Button A**: GPIO0 (Reset altitude to zero)
- **Button B**: GPIO47 (Toggle display mode)
- **Button C**: GPIO48 (Toggle display on/off)
- **RGB LED**: GPIO8 (Data), GPIO7 (Power)

## Display Modes

### Mode 1: Main Overview
- Current altitude (large, green text)
- Maximum altitude (red text) - **Always visible**
- Temperature (yellow text)
- Pressure (cyan text)
- Sensor status indicator

### Mode 2: Detailed View
- Current altitude (large, with 2 decimal places)
- Maximum altitude (with 2 decimal places) - **Always visible**
- Difference from maximum altitude (color-coded)
- Sensor status indicator

## Controls

### Button Functions
- **Button A (GPIO0)**: Reset altitude to zero - Sets current location as reference point
- **Button B (GPIO47)**: Toggle between display modes (Main/Detailed)
- **Button C (GPIO48)**: Toggle display on/off for power saving

### LED Status Indicators
- **Green Breathing**: BMP180 sensor working properly
- **Red Breathing**: Sensor error or connection issue
- **Solid Colors**: Button press feedback (Orange/Blue/Green/Red)

## WiFi Web Interface

The device creates a WiFi access point for remote monitoring:

- **SSID**: `Altimeter-S3`
- **Password**: `altimeter123`
- **Web Interface**: Connect to WiFi and visit the device's IP address

### Web Features
- Real-time altitude, temperature, and pressure readings
- Maximum altitude display
- Remote altitude reset
- Display toggle control
- Sensor status monitoring

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
- **Method**: Barometric pressure differential
- **Reference**: Standard sea level pressure (1013.25 hPa)
- **Accuracy**: ±1-2 meters (typical)
- **Resolution**: 0.1 meters
- **Update Rate**: 5 Hz sensor readings, 2 Hz display updates

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
Pure Altimeter - No Test Mode
Board: LOLIN S3 Mini Pro
Display: 0.85" 128x128 TFT (ST7789)
Sensor: BMP180 Pressure/Temperature
========================================
✓ Buttons initialized
✓ RGB LED initialized
✓ TFT display initialized
✓ BMP180 sensor initialized successfully
✓ Current pressure: 1013.25 hPa
✓ Using sea level baseline: 1013.25 hPa
✓ WiFi and Web Server ready
========================================
🚀 ALTIMETER READY!
========================================
Controls:
  Button A (GPIO0)  - Reset altitude to zero
  Button B (GPIO47) - Toggle display mode
  Button C (GPIO48) - Toggle display on/off
========================================
ALT: 0.0m (MAX: 0.0m) | TEMP: 23.5°C | PRESS: 1013.2 hPa
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
   - Use Button A to reset altitude to zero at known location
   - Allow sensor to stabilize for 30 seconds after power-on
   - Verify local atmospheric pressure if absolute accuracy needed

4. **WiFi Connection Issues**
   - Look for "Altimeter-S3" network
   - Use password "altimeter123"
   - Check serial monitor for IP address

### Build Errors
- Ensure all dependencies are installed via PlatformIO
- Check that simple_font.h is present in src/ directory
- Verify correct board selection (lolin_s3_mini)

## Project Structure

```
TripleT-Altimetre-v1/
├── src/
│   ├── main.cpp              # Main altimeter application
│   ├── main_tft_test.cpp     # TFT test application
│   ├── tft_test.cpp          # Custom TFT driver
│   ├── tft_test.h            # TFT driver header
│   └── simple_font.h         # Bitmap font for display
├── include/
├── lib/
├── platformio.ini            # Build configuration
├── User_Setup.h              # TFT_eSPI configuration
└── README.md                 # This file
```

## Version History

- **v2.1**: Simplified altimeter with focus on sensor data display
  - Removed all test functionality from main application
  - Clean, dedicated altimeter interface
  - Maximum altitude always visible on display
  - Improved power management with display control
  - Enhanced web interface for remote monitoring

- **v2.0**: Complete altimeter with multi-mode display
- **v1.0**: Initial TFT display and sensor integration

## License

This project is open source. Feel free to modify and distribute according to your needs.

## Support

For issues or questions, check the serial monitor output for diagnostic information. The device provides detailed status messages and error reporting to help with troubleshooting.


