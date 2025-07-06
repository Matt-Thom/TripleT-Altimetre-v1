# TripleT Altimeter v1

A comprehensive altimeter system built on the LOLIN S3 Mini Pro ESP32-S3 development board with a 0.85" TFT display, BMP180 pressure sensor, and optional IMU.

## Hardware Configuration

### Board: LOLIN S3 Mini Pro
- **MCU**: ESP32-S3 (240MHz, 320KB RAM, 4MB Flash)
- **Display**: 0.85" 128x128 TFT (ST7789 driver)
- **Sensors**: BMP180 pressure/temperature sensor
- **Optional**: QMI8658C IMU (6-axis accelerometer/gyroscope)
- **RGB LED**: WS2812B for status indication
- **Buttons**: 3 tactile buttons for user interaction

### Pin Configuration
```
TFT Display (SPI):
- CS:   GPIO35
- DC:   GPIO36  
- RST:  GPIO34
- BL:   GPIO33 (backlight)
- MOSI: GPIO38
- SCLK: GPIO40
- MISO: GPIO39

I2C Sensors:
- SDA:  GPIO12
- SCL:  GPIO11

RGB LED:
- DATA: GPIO8
- POWER: GPIO7

Buttons:
- Button A: GPIO0  (Calibrate altitude)
- Button B: GPIO47 (Change display mode)
- Button C: GPIO48 (Force refresh)
```

## Features

### 🎯 **Altitude Measurement**
- Real-time altitude calculation using BMP180 pressure sensor
- Calibration feature to set current location as 0m reference
- Maximum altitude tracking
- Pressure and temperature readings

### 📱 **Multi-Mode Display**
- **Overview Mode**: All key data on one screen
- **Altitude Detail**: Focused altitude information
- **Environmental**: Temperature and pressure details
- **IMU Mode**: Accelerometer and gyroscope data (if available)

### 🎨 **Visual Interface**
- Custom 5x7 bitmap font for crisp, readable text
- Color-coded data display
- Status indicators for sensor health
- Bar graphs for visual data representation
- Breathing LED status indication

### 🔧 **User Controls**
- **Button A**: Calibrate altitude (set current location to 0m)
- **Button B**: Cycle through display modes
- **Button C**: Force display refresh

### 📊 **Data Visualization**
- Current altitude with decimal precision
- Maximum altitude reached
- Temperature in Celsius
- Pressure in hPa
- IMU data (acceleration and gyroscope)
- Visual bar graphs for key metrics

## Software Architecture

### Core Components
- **`main_altimeter.cpp`**: Main application logic and sensor management
- **`altimeter_display.cpp/.h`**: Display interface and rendering
- **`simple_font.h`**: Bitmap font system for text rendering
- **`tft_test.cpp/.h`**: Low-level TFT driver (ST7789)
- **`imu_simulator.cpp/.h`**: IMU data simulation for testing

### Display System
- Custom bitmap font rendering for excellent readability
- Efficient partial screen updates
- Color-coded information display
- Multi-mode interface with smooth transitions

## Build and Upload

### Using PlatformIO

1. **Build the altimeter**:
   ```bash
   pio run -e lolin_s3_mini_altimeter
   ```

2. **Upload to device**:
   ```bash
   pio run -e lolin_s3_mini_altimeter -t upload
   ```

3. **Monitor serial output**:
   ```bash
   pio device monitor -e lolin_s3_mini_altimeter
   ```

### Alternative Builds

- **Original main.cpp**: `pio run -e lolin_s3_mini_pro`
- **TFT Test driver**: `pio run -e lolin_s3_mini_tft_test`

## Usage

### Initial Setup
1. Power on the device
2. Wait for sensor initialization (LED will indicate status)
3. Calibrate altitude by pressing Button A at your reference location

### Operation
- **Green LED**: BMP180 sensor working
- **Cyan LED**: Both BMP180 and IMU working  
- **Red LED**: Sensor issues
- **Breathing effect**: Normal operation

### Calibration
Press Button A to set the current location as 0m altitude reference. This recalibrates the pressure baseline for accurate relative altitude measurements.

## Display Modes

### 1. Overview Mode
- Current altitude (large display)
- Maximum altitude reached
- Temperature and pressure (compact)
- Altitude bar graph
- IMU acceleration magnitude (if available)

### 2. Altitude Detail Mode
- Current altitude (extra large)
- Maximum altitude
- Difference from maximum
- Detailed precision display

### 3. Environmental Mode
- Temperature with bar graph
- Pressure with bar graph
- Extended range displays

### 4. IMU Mode
- 3-axis acceleration data
- 3-axis gyroscope data
- Real-time sensor readings
- Connection status

## Technical Specifications

### Sensor Accuracy
- **Altitude**: ±1m resolution (BMP180 dependent)
- **Temperature**: ±0.5°C accuracy
- **Pressure**: ±0.12 hPa accuracy
- **Update Rate**: 5Hz sensor readings, 10Hz display updates

### Display Performance
- **Resolution**: 128x128 pixels
- **Colors**: 16-bit RGB565
- **Font**: Custom 5x7 bitmap font
- **Refresh Rate**: Optimized partial updates

### Power Consumption
- **Active**: ~80mA @ 3.3V
- **Display**: ~30mA (backlight dependent)
- **Sensors**: ~5mA combined

## Troubleshooting

### Common Issues

1. **Altitude shows incorrect values**:
   - Press Button A to calibrate at current location
   - Ensure BMP180 is properly connected
   - Check I2C connections (SDA=GPIO12, SCL=GPIO11)

2. **Display not working**:
   - Verify SPI connections
   - Check TFT power and backlight
   - Ensure correct pin configuration

3. **Sensor errors**:
   - Check I2C wiring
   - Verify 3.3V power supply
   - Look for loose connections

### Serial Debug Output
Connect to serial monitor at 115200 baud for detailed diagnostic information including:
- Sensor initialization status
- Real-time altitude readings
- Button press events
- Error messages

## Development

### Adding New Features
The modular design allows easy extension:
- Add new display modes in `altimeter_display.cpp`
- Extend sensor support in `main_altimeter.cpp`
- Modify font or graphics in `simple_font.h`

### Testing
- IMU simulator provides realistic data for testing
- TFT test driver available for display debugging
- Serial output provides comprehensive diagnostics

## License

This project is open source. Feel free to modify and distribute according to your needs.

## Version History

- **v2.1**: Complete altimeter with improved display and calibration
- **v2.0**: TFT display integration and basic altimeter functions
- **v1.0**: Initial BMP180 sensor integration

---

**Status**: ✅ **FULLY FUNCTIONAL** - Ready for field testing and use


