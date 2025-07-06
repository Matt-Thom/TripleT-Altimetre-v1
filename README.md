# TripleT Altimeter v2.0 - LOLIN S3 Mini Pro

A professional-grade altimeter project built on the LOLIN S3 Mini Pro development board, featuring a built-in TFT display, IMU sensor, and comprehensive data logging capabilities.

## Hardware Specifications

### LOLIN S3 Mini Pro Board
- **MCU**: ESP32-S3FH4R2
- **Connectivity**: 2.4 GHz Wi-Fi + Bluetooth LE
- **Memory**: 4MB Flash + 2MB PSRAM
- **Display**: 0.85" 128x128 LCD TFT (GC9107/GC9A01)
- **IMU**: QMI8658C 6D MEMS sensor
- **I/O**: 12x GPIO pins
- **Buttons**: 3x programmable buttons (GPIO0, GPIO47, GPIO48)
- **LED**: 1x RGB LED (Data: GPIO8, Power: GPIO7)
- **IR**: GPIO9
- **Size**: 34.3mm × 25.4mm
- **Compatibility**: LOLIN D1 Mini shields

### External Sensors
- **BMP180**: Pressure/temperature sensor for altitude calculation
- **Battery monitoring**: Via ADC on GPIO1

## Features

### Core Functionality
- **Real-time altitude measurement** with BMP180 sensor
- **Maximum altitude tracking** with reset capability
- **Multi-screen TFT display** with three different views
- **IMU integration** for orientation sensing (pitch/roll)
- **Data logging** to SPIFFS filesystem
- **Battery voltage monitoring**
- **RGB LED status indicators**

### Display Screens
1. **Main Screen**: Current altitude, max altitude, battery status
2. **Sensor Screen**: Temperature, pressure, IMU data
3. **Settings Screen**: System information, controls, uptime

### User Interface
- **Button 1 (GPIO0)**: Reset maximum altitude
- **Button 2 (GPIO47)**: Cycle through display screens
- **Button 3 (GPIO48)**: Toggle logging (placeholder)
- **RGB LED**: Visual feedback for button presses and system status

## Pin Configuration

### Built-in Components
```
Display (TFT):
- MOSI: GPIO3
- SCLK: GPIO2
- CS: GPIO1
- DC: GPIO4
- RST: GPIO5
- BL: GPIO6

RGB LED:
- Data: GPIO8
- Power: GPIO7

Buttons:
- Button 1: GPIO0
- Button 2: GPIO47
- Button 3: GPIO48

Other:
- IR: GPIO9
- Battery ADC: GPIO1
```

### External Sensors
```
BMP180 (I2C):
- SDA: Default I2C pins
- SCL: Default I2C pins
- VCC: 3.3V
- GND: GND
```

## Software Architecture

### Libraries Used
- **TFT_eSPI**: Display driver for GC9A01 TFT
- **Adafruit BMP085**: Pressure sensor library
- **SPIFFS**: File system for data logging
- **WiFi**: Network connectivity (future use)

### Data Logging
Data is logged to `/altimeter_data.csv` with the following format:
```
Timestamp,Temperature_C,Pressure_Pa,Altitude_m,AccelX,AccelY,AccelZ,Pitch,Roll,Yaw,Battery_V
```

### Display System
- **128x128 pixel circular TFT display**
- **Color-coded interface** (Green: normal, Red: warnings, Blue: info)
- **Multi-screen navigation** with button controls
- **Real-time updates** at 10Hz display refresh

## Setup Instructions

### 1. Hardware Assembly
1. Connect BMP180 sensor to I2C pins
2. Ensure proper power connections
3. Verify all connections are secure

### 2. Software Setup
1. Install PlatformIO IDE
2. Clone this repository
3. Open project in PlatformIO
4. Build and upload firmware

### 3. Configuration
The project is pre-configured for LOLIN S3 Mini Pro. Key settings:
- **Board**: `lolin_s3_mini`
- **PSRAM**: Enabled (2MB)
- **USB CDC**: Enabled for serial communication
- **Display**: GC9A01 driver with 128x128 resolution

## Build Configuration

### PlatformIO Settings
```ini
[env:lolin_s3_mini_pro]
platform = espressif32
board = lolin_s3_mini
framework = arduino
build_flags = 
    -DBOARD_HAS_PSRAM=1
    -DARDUINO_USB_CDC_ON_BOOT=1
    -DARDUINO_USB_MODE=1
    -DUSER_SETUP_LOADED=1
    -DGC9A01_DRIVER=1
    -DTFT_WIDTH=128
    -DTFT_HEIGHT=128
```

### Memory Configuration
- **Flash**: 4MB (sufficient for firmware and data logging)
- **PSRAM**: 2MB (enabled for enhanced performance)
- **Heap**: Dynamic allocation for display buffers

## Usage

### Basic Operation
1. Power on the device
2. Wait for sensor initialization
3. Use buttons to navigate between screens
4. Monitor altitude readings on main screen
5. Press Button 1 to reset maximum altitude

### Data Logging
- Data is automatically logged every 5 seconds
- Log files are stored in SPIFFS filesystem
- Access logs via serial interface or future web interface

### Troubleshooting

#### Common Issues
1. **Display not working**: Check TFT connections and power
2. **BMP180 not found**: Verify I2C connections
3. **IMU not detected**: Check I2C address and connections
4. **No serial output**: Ensure USB CDC is enabled

#### Debug Information
Enable debug output by setting `CORE_DEBUG_LEVEL=3` in build flags.

## Development Status

### Completed Features ✅
- [x] TFT display integration
- [x] Multi-screen interface
- [x] BMP180 sensor integration
- [x] Button controls
- [x] RGB LED feedback
- [x] Data logging system
- [x] Battery monitoring
- [x] IMU framework (basic)

### Future Enhancements 🚧
- [ ] Complete IMU sensor implementation
- [ ] WiFi connectivity for data sync
- [ ] Web interface for data visualization
- [ ] Advanced altitude calculations
- [ ] GPS integration
- [ ] Weather station features

## Technical Details

### Performance
- **Sensor Reading**: 2Hz (500ms intervals)
- **Display Update**: 10Hz (100ms intervals)
- **Data Logging**: 0.2Hz (5-second intervals)
- **Button Response**: <50ms debounce

### Power Management
- **Sleep modes**: Not implemented (always-on display)
- **Battery monitoring**: Real-time voltage measurement
- **Power consumption**: ~200mA (estimated with display on)

## Version History

### v2.0 (Current)
- Complete refactor for LOLIN S3 Mini Pro
- Built-in TFT display support
- IMU sensor integration
- Enhanced user interface
- Improved data logging

### v1.0 (Previous)
- Basic BMP180 altimeter
- Serial-only interface
- LOLIN S3 Mini compatibility
- Basic data logging

## License

This project is open-source and available under the MIT License.

## Contributing

Contributions are welcome! Please feel free to submit issues, feature requests, or pull requests.

## Support

For support and questions:
1. Check the troubleshooting section
2. Review the code comments
3. Open an issue on GitHub
4. Contact the development team

---

**Built with ❤️ for the maker community**

Last updated: January 2025


