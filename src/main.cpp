#include <Arduino.h>
#include <Wire.h>
// #include <TFT_eSPI.h>  // Temporarily disabled due to pin configuration issues
#include <Adafruit_BMP085.h>
#include "FS.h"
#include "SPIFFS.h"
#include <WiFi.h>

// Hardware configuration for LOLIN S3 Mini Pro
#define RGB_LED_DATA_PIN 8
#define RGB_LED_POWER_PIN 7
#define IR_PIN 9
#define BUTTON_1_PIN 0
#define BUTTON_2_PIN 47
#define BUTTON_3_PIN 48
#define BATTERY_PIN 1

// I2C pins for LOLIN S3 Mini Pro - BMP180 sensor
#define I2C_SDA_PIN 12  // SDA for BMP180
#define I2C_SCL_PIN 11  // SCL for BMP180

// IMU I2C address (QMI8658C)
#define QMI8658_I2C_ADDR 0x6B

// RGB LED Colors
#define LED_OFF        0x000000
#define LED_RED        0xFF0000
#define LED_GREEN      0x00FF00
#define LED_BLUE       0x0000FF
#define LED_YELLOW     0xFFFF00
#define LED_PURPLE     0xFF00FF
#define LED_CYAN       0x00FFFF
#define LED_WHITE      0xFFFFFF
#define LED_ORANGE     0xFF8000

// Display and sensor objects
// TFT_eSPI tft = TFT_eSPI();  // Temporarily disabled
Adafruit_BMP085 bmp180;

// Global variables
float maxAltitude = 0.0;
float currentAltitude = 0.0;
float temperature = 0.0;
float pressure = 0.0;
float batteryVoltage = 0.0;
bool sensorInitialized = false;
bool imuInitialized = false;
bool displayInitialized = false;

// IMU data
float accelX = 0.0, accelY = 0.0, accelZ = 0.0;
float gyroX = 0.0, gyroY = 0.0, gyroZ = 0.0;
float pitch = 0.0, roll = 0.0, yaw = 0.0;

// Button states
unsigned long lastButtonPressTime = 0;
const unsigned long debounceDelay = 50;
int currentScreen = 0; // 0=main, 1=sensors, 2=settings
const int maxScreens = 3;

// Data logging
const char* dataLogFile = "/altimeter_data.csv";
unsigned long lastLogTime = 0;
const unsigned long logInterval = 5000; // Log every 5 seconds

// Status LED management
unsigned long lastStatusUpdate = 0;
const unsigned long statusUpdateInterval = 1000; // Update status LED every second
int statusLEDState = 0;

// Function prototypes
void initDisplay();
void initSensors();
void initIMU();
void initSPIFFS();
void initButtons();
void initRGBLED();
void readSensors();
void updateDisplay();
void handleButtons();
void logData();
void displaySerialData();
void setRGBLED(uint32_t color);
void setRGBLEDRaw(uint8_t r, uint8_t g, uint8_t b);
void updateStatusLED();
void flashLED(uint32_t color, int duration);
float readBatteryVoltage();
void calculateOrientation();
bool checkIMUConnection();
void scanI2CDevices();

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("===== LOLIN S3 Mini Pro Altimeter v2.0 =====");
    Serial.println("Board: LOLIN S3 Mini Pro");
    Serial.println("Display: 0.85\" 128x128 TFT (DISABLED - Pin Config Issue)");
    Serial.println("IMU: QMI8658C 6D MEMS");
    Serial.println("Flash: 4MB, PSRAM: 2MB");
    Serial.println("I2C: SDA=GPIO12, SCL=GPIO11 (BMP180)");
    Serial.println("RGB LED: Data=GPIO8, Power=GPIO7");
    Serial.println("==========================================");
    
    // Initialize hardware components
    initDisplay();
    initButtons();
    initRGBLED();
    
    // Show initialization status
    setRGBLED(LED_YELLOW); // Yellow = initializing
    
    initSPIFFS();
    initSensors();
    initIMU();
    
    Serial.println("=== Altimeter Ready ===");
    Serial.println("Button 1 (GPIO0): Reset max altitude");
    Serial.println("Button 2 (GPIO47): Change display mode");
    Serial.println("Button 3 (GPIO48): Toggle logging");
    Serial.println("Using Serial Interface (TFT temporarily disabled)");
    Serial.println("====================================");
    Serial.println("LED Status Colors:");
    Serial.println("- Green: System ready, sensor working");
    Serial.println("- Red: Sensor error");
    Serial.println("- Blue: Button pressed");
    Serial.println("- Yellow: Initializing");
    Serial.println("- Purple: Logging data");
    Serial.println("- Cyan: IMU active");
    Serial.println("====================================");
    
    // Set initial status LED
    if (sensorInitialized) {
        setRGBLED(LED_GREEN); // Green = ready with sensor
    } else {
        setRGBLED(LED_RED); // Red = sensor error
    }
    
    delay(1000);
}

void loop() {
    static unsigned long lastSensorRead = 0;
    static unsigned long lastDisplayUpdate = 0;
    const unsigned long sensorInterval = 500;   // Read sensors every 500ms
    const unsigned long displayInterval = 2000; // Update display every 2 seconds
    
    unsigned long currentTime = millis();
    
    // Read sensors
    if (currentTime - lastSensorRead >= sensorInterval) {
        readSensors();
        lastSensorRead = currentTime;
    }
    
    // Update display (serial output)
    if (currentTime - lastDisplayUpdate >= displayInterval) {
        updateDisplay();
        lastDisplayUpdate = currentTime;
    }
    
    // Update status LED
    if (currentTime - lastStatusUpdate >= statusUpdateInterval) {
        updateStatusLED();
        lastStatusUpdate = currentTime;
    }
    
    // Handle button presses
    handleButtons();
    
    // Log data periodically
    if (currentTime - lastLogTime >= logInterval) {
        flashLED(LED_PURPLE, 200); // Purple flash when logging
        logData();
        lastLogTime = currentTime;
    }
    
    delay(10);
}

void initDisplay() {
    Serial.println("TFT display initialization skipped (pin configuration issue)");
    displayInitialized = false; // Keep false to use serial interface
    Serial.println("Using serial interface for data display");
}

void initSensors() {
    Serial.println("Initializing BMP180 sensor...");
    
    // Initialize I2C with specific pins for BMP180
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Serial.print("I2C initialized with SDA=GPIO");
    Serial.print(I2C_SDA_PIN);
    Serial.print(", SCL=GPIO");
    Serial.print(I2C_SCL_PIN);
    Serial.println(" (for BMP180)");
    
    // Scan for I2C devices
    scanI2CDevices();
    
    // Try to initialize BMP180
    if (bmp180.begin()) {
        sensorInitialized = true;
        Serial.println("BMP180 sensor initialized successfully!");
        flashLED(LED_GREEN, 500); // Green flash for success
    } else {
        Serial.println("BMP180 sensor not found - check I2C connections");
        Serial.println("Expected connections: SDA→GPIO12, SCL→GPIO11, VCC→3.3V, GND→GND");
        sensorInitialized = false;
        flashLED(LED_RED, 500); // Red flash for error
    }
}

void scanI2CDevices() {
    Serial.println("Scanning I2C bus for devices...");
    byte error, address;
    int nDevices = 0;
    
    for (address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        
        if (error == 0) {
            Serial.print("I2C device found at address 0x");
            if (address < 16) Serial.print("0");
            Serial.print(address, HEX);
            Serial.println();
            nDevices++;
        }
    }
    
    if (nDevices == 0) {
        Serial.println("No I2C devices found");
    } else {
        Serial.print("Found ");
        Serial.print(nDevices);
        Serial.println(" I2C device(s)");
    }
    Serial.println("I2C scan complete");
}

void initIMU() {
    Serial.println("Initializing QMI8658C IMU...");
    
    // Check if IMU is connected
    if (checkIMUConnection()) {
        imuInitialized = true;
        Serial.println("QMI8658C IMU detected on I2C bus");
        flashLED(LED_CYAN, 500); // Cyan flash for IMU detected
        
        // Basic IMU initialization would go here
        // For now, we'll just mark it as initialized
        
    } else {
        Serial.println("QMI8658C IMU not found on I2C bus");
        Serial.println("This is expected if IMU is not connected or uses different address");
        imuInitialized = false;
    }
}

bool checkIMUConnection() {
    Wire.beginTransmission(QMI8658_I2C_ADDR);
    byte error = Wire.endTransmission();
    return (error == 0);
}

void initSPIFFS() {
    Serial.println("Initializing SPIFFS...");
    
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS initialization failed");
        return;
    }
    
    // Create log file with headers if it doesn't exist
    if (!SPIFFS.exists(dataLogFile)) {
        File file = SPIFFS.open(dataLogFile, FILE_WRITE);
        if (file) {
            file.println("Timestamp,Temperature_C,Pressure_Pa,Altitude_m,AccelX,AccelY,AccelZ,Pitch,Roll,Yaw,Battery_V");
            file.close();
            Serial.println("Created new log file with headers");
        }
    } else {
        Serial.println("Log file already exists");
    }
    
    Serial.println("SPIFFS initialized successfully");
}

void initButtons() {
    pinMode(BUTTON_1_PIN, INPUT_PULLUP);
    pinMode(BUTTON_2_PIN, INPUT_PULLUP);
    pinMode(BUTTON_3_PIN, INPUT_PULLUP);
    Serial.println("Buttons initialized (GPIO0, GPIO47, GPIO48)");
}

void initRGBLED() {
    pinMode(RGB_LED_POWER_PIN, OUTPUT);
    pinMode(RGB_LED_DATA_PIN, OUTPUT);
    digitalWrite(RGB_LED_POWER_PIN, HIGH); // Enable LED power
    Serial.println("RGB LED initialized (Data: GPIO8, Power: GPIO7)");
    
    // Test LED colors
    Serial.println("Testing RGB LED colors...");
    setRGBLED(LED_RED);
    delay(200);
    setRGBLED(LED_GREEN);
    delay(200);
    setRGBLED(LED_BLUE);
    delay(200);
    setRGBLED(LED_OFF);
}

void readSensors() {
    // Read BMP180 if available
    if (sensorInitialized) {
        temperature = bmp180.readTemperature();
        pressure = bmp180.readPressure();
        currentAltitude = bmp180.readAltitude();
        
        // Update max altitude
        if (currentAltitude > maxAltitude) {
            maxAltitude = currentAltitude;
        }
    }
    
    // Read IMU if available (placeholder for now)
    if (imuInitialized) {
        // Placeholder IMU reading - would need proper library implementation
        // For now, generate some dummy data to test the interface
        accelX = 0.0;
        accelY = 0.0;
        accelZ = 1.0; // 1g downward
        calculateOrientation();
    }
    
    // Read battery voltage
    batteryVoltage = readBatteryVoltage();
}

void calculateOrientation() {
    // Simple orientation calculation from accelerometer
    if (imuInitialized) {
        pitch = atan2(accelY, sqrt(accelX * accelX + accelZ * accelZ)) * 180.0 / PI;
        roll = atan2(-accelX, accelZ) * 180.0 / PI;
        // Yaw requires magnetometer or integration
        yaw = 0.0; // Placeholder
    }
}

void updateDisplay() {
    // Use serial interface since TFT is disabled
    displaySerialData();
}

void displaySerialData() {
    Serial.println("\n=== LOLIN S3 Mini Pro Altimeter Data ===");
    
    // Current status
    Serial.print("Current Altitude: ");
    if (sensorInitialized) {
        Serial.print(currentAltitude, 1);
        Serial.println(" m");
    } else {
        Serial.println("NO SENSOR");
    }
    
    Serial.print("Maximum Altitude: ");
    Serial.print(maxAltitude, 1);
    Serial.println(" m");
    
    if (sensorInitialized) {
        Serial.print("Temperature: ");
        Serial.print(temperature, 1);
        Serial.println(" °C");
        
        Serial.print("Pressure: ");
        Serial.print(pressure / 100.0, 1);
        Serial.println(" hPa");
    }
    
    // IMU data
    if (imuInitialized) {
        Serial.println("IMU: ACTIVE");
        Serial.print("Pitch: ");
        Serial.print(pitch, 1);
        Serial.print("°, Roll: ");
        Serial.print(roll, 1);
        Serial.println("°");
    } else {
        Serial.println("IMU: NOT DETECTED");
    }
    
    // System info
    Serial.print("Battery Voltage: ");
    Serial.print(batteryVoltage, 2);
    Serial.println(" V");
    
    Serial.print("Free Heap: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");
    
    Serial.print("Uptime: ");
    Serial.print(millis() / 1000);
    Serial.println(" seconds");
    
    Serial.println("Press Button 1 (GPIO0) to reset max altitude");
    Serial.println("==========================================");
}

void updateStatusLED() {
    // Breathing effect for status LED
    static int brightness = 0;
    static int direction = 1;
    
    brightness += direction * 5;
    if (brightness >= 255) {
        brightness = 255;
        direction = -1;
    } else if (brightness <= 0) {
        brightness = 0;
        direction = 1;
    }
    
    // Choose color based on system status
    if (sensorInitialized && imuInitialized) {
        // Both sensors working - cyan breathing
        setRGBLEDRaw(0, brightness/4, brightness/4);
    } else if (sensorInitialized) {
        // Only BMP180 working - green breathing
        setRGBLEDRaw(0, brightness/4, 0);
    } else {
        // No sensors - red breathing
        setRGBLEDRaw(brightness/4, 0, 0);
    }
}

void handleButtons() {
    unsigned long currentTime = millis();
    
    // Button 1: Reset max altitude
    if (digitalRead(BUTTON_1_PIN) == LOW) {
        if (currentTime - lastButtonPressTime > debounceDelay) {
            maxAltitude = currentAltitude;
            Serial.println("\n*** MAX ALTITUDE RESET ***");
            Serial.print("Max altitude reset to: ");
            Serial.print(maxAltitude, 1);
            Serial.println(" m");
            flashLED(LED_ORANGE, 300); // Orange flash for reset
            lastButtonPressTime = currentTime;
        }
    }
    
    // Button 2: Change display mode (placeholder)
    if (digitalRead(BUTTON_2_PIN) == LOW) {
        if (currentTime - lastButtonPressTime > debounceDelay) {
            currentScreen = (currentScreen + 1) % maxScreens;
            Serial.print("\n*** DISPLAY MODE CHANGED TO: ");
            Serial.print(currentScreen);
            Serial.println(" ***");
            flashLED(LED_BLUE, 300); // Blue flash for mode change
            lastButtonPressTime = currentTime;
        }
    }
    
    // Button 3: Toggle logging (placeholder)
    if (digitalRead(BUTTON_3_PIN) == LOW) {
        if (currentTime - lastButtonPressTime > debounceDelay) {
            Serial.println("\n*** LOGGING TOGGLE (placeholder) ***");
            flashLED(LED_YELLOW, 300); // Yellow flash for logging toggle
            lastButtonPressTime = currentTime;
        }
    }
}

void logData() {
    File file = SPIFFS.open(dataLogFile, FILE_APPEND);
    if (!file) {
        Serial.println("Failed to open log file");
        return;
    }
    
    String logEntry = String(millis()) + "," +
                     String(temperature, 2) + "," +
                     String(pressure, 2) + "," +
                     String(currentAltitude, 2) + "," +
                     String(accelX, 3) + "," +
                     String(accelY, 3) + "," +
                     String(accelZ, 3) + "," +
                     String(pitch, 2) + "," +
                     String(roll, 2) + "," +
                     String(yaw, 2) + "," +
                     String(batteryVoltage, 3);
    
    file.println(logEntry);
    file.close();
    
    Serial.println("Data logged: " + logEntry);
}

void setRGBLED(uint32_t color) {
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    setRGBLEDRaw(r, g, b);
}

void setRGBLEDRaw(uint8_t r, uint8_t g, uint8_t b) {
    // Simple RGB LED control using PWM
    // This assumes a common cathode RGB LED
    analogWrite(RGB_LED_DATA_PIN, r > 0 ? 255 : 0); // Simple on/off for now
    // For proper RGB control, you'd need 3 separate pins or a smart LED like WS2812
}

void flashLED(uint32_t color, int duration) {
    setRGBLED(color);
    delay(duration);
    setRGBLED(LED_OFF);
}

float readBatteryVoltage() {
    int adcValue = analogRead(BATTERY_PIN);
    float voltage = (adcValue / 4095.0) * 3.3 * 2.0; // Assuming voltage divider
    return voltage;
}