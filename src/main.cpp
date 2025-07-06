#include <Arduino.h>
#include <Wire.h>
#include <TFT_eSPI.h>
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

// IMU I2C address (QMI8658C)
#define QMI8658_I2C_ADDR 0x6B

// Display and sensor objects
TFT_eSPI tft = TFT_eSPI();
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

// Display colors
#define COLOR_BG       0x0000    // Black
#define COLOR_TEXT     0xFFFF    // White
#define COLOR_ACCENT   0x07E0    // Green
#define COLOR_WARNING  0xF800    // Red
#define COLOR_INFO     0x001F    // Blue

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
void displayMainScreen();
void displaySensorScreen();
void displaySettingsScreen();
void setRGBLED(uint8_t r, uint8_t g, uint8_t b);
float readBatteryVoltage();
void calculateOrientation();
bool checkIMUConnection();

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("===== LOLIN S3 Mini Pro Altimeter v2.0 =====");
    Serial.println("Board: LOLIN S3 Mini Pro");
    Serial.println("Display: 0.85\" 128x128 TFT (GC9A01)");
    Serial.println("IMU: QMI8658C 6D MEMS");
    Serial.println("Flash: 4MB, PSRAM: 2MB");
    Serial.println("==========================================");
    
    // Initialize hardware components
    initDisplay();
    initButtons();
    initRGBLED();
    initSPIFFS();
    initSensors();
    initIMU();
    
    Serial.println("=== Altimeter Ready ===");
    Serial.println("Button 1 (GPIO0): Reset max altitude");
    Serial.println("Button 2 (GPIO47): Change screen");
    Serial.println("Button 3 (GPIO48): Toggle logging");
    
    // Set initial LED color (green = ready)
    setRGBLED(0, 255, 0);
    delay(500);
    setRGBLED(0, 0, 0); // Turn off
}

void loop() {
    static unsigned long lastSensorRead = 0;
    static unsigned long lastDisplayUpdate = 0;
    const unsigned long sensorInterval = 500;   // Read sensors every 500ms
    const unsigned long displayInterval = 100;  // Update display every 100ms
    
    unsigned long currentTime = millis();
    
    // Read sensors
    if (currentTime - lastSensorRead >= sensorInterval) {
        readSensors();
        lastSensorRead = currentTime;
    }
    
    // Update display
    if (currentTime - lastDisplayUpdate >= displayInterval) {
        updateDisplay();
        lastDisplayUpdate = currentTime;
    }
    
    // Handle button presses
    handleButtons();
    
    // Log data periodically
    if (currentTime - lastLogTime >= logInterval) {
        logData();
        lastLogTime = currentTime;
    }
    
    delay(10);
}

void initDisplay() {
    Serial.println("Initializing TFT display...");
    
    // Initialize display
    tft.init();
    tft.setRotation(0);
    tft.fillScreen(COLOR_BG);
    tft.setTextColor(COLOR_TEXT, COLOR_BG);
    tft.setTextSize(1);
    
    // Show startup screen
    tft.setCursor(10, 10);
    tft.println("LOLIN S3 Mini Pro");
    tft.setCursor(10, 30);
    tft.println("Altimeter v2.0");
    tft.setCursor(10, 50);
    tft.println("Initializing...");
    
    displayInitialized = true;
    Serial.println("TFT display initialized");
}

void initSensors() {
    Serial.println("Initializing BMP180 sensor...");
    
    // Initialize I2C
    Wire.begin();
    
    // Try to initialize BMP180
    if (bmp180.begin()) {
        sensorInitialized = true;
        Serial.println("BMP180 sensor initialized successfully");
    } else {
        Serial.println("BMP180 sensor not found - continuing without pressure sensor");
        sensorInitialized = false;
    }
}

void initIMU() {
    Serial.println("Initializing QMI8658C IMU...");
    
    // Check if IMU is connected
    if (checkIMUConnection()) {
        imuInitialized = true;
        Serial.println("QMI8658C IMU detected");
        
        // Basic IMU initialization would go here
        // For now, we'll just mark it as initialized
        
    } else {
        Serial.println("QMI8658C IMU not found - continuing without IMU");
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
    }
    
    Serial.println("SPIFFS initialized");
}

void initButtons() {
    pinMode(BUTTON_1_PIN, INPUT_PULLUP);
    pinMode(BUTTON_2_PIN, INPUT_PULLUP);
    pinMode(BUTTON_3_PIN, INPUT_PULLUP);
    Serial.println("Buttons initialized");
}

void initRGBLED() {
    pinMode(RGB_LED_POWER_PIN, OUTPUT);
    pinMode(RGB_LED_DATA_PIN, OUTPUT);
    digitalWrite(RGB_LED_POWER_PIN, HIGH); // Enable LED power
    Serial.println("RGB LED initialized");
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
    if (!displayInitialized) return;
    
    switch (currentScreen) {
        case 0:
            displayMainScreen();
            break;
        case 1:
            displaySensorScreen();
            break;
        case 2:
            displaySettingsScreen();
            break;
    }
}

void displayMainScreen() {
    tft.fillScreen(COLOR_BG);
    tft.setTextColor(COLOR_TEXT, COLOR_BG);
    
    // Title
    tft.setTextSize(2);
    tft.setCursor(10, 5);
    tft.setTextColor(COLOR_ACCENT, COLOR_BG);
    tft.println("ALTIMETER");
    
    // Current altitude
    tft.setTextSize(1);
    tft.setTextColor(COLOR_TEXT, COLOR_BG);
    tft.setCursor(5, 30);
    tft.println("Current Alt:");
    tft.setTextSize(2);
    tft.setCursor(5, 45);
    if (sensorInitialized) {
        tft.printf("%.1f m", currentAltitude);
    } else {
        tft.setTextColor(COLOR_WARNING, COLOR_BG);
        tft.println("NO SENSOR");
    }
    
    // Max altitude
    tft.setTextSize(1);
    tft.setTextColor(COLOR_TEXT, COLOR_BG);
    tft.setCursor(5, 70);
    tft.println("Max Alt:");
    tft.setTextSize(2);
    tft.setCursor(5, 85);
    tft.setTextColor(COLOR_ACCENT, COLOR_BG);
    tft.printf("%.1f m", maxAltitude);
    
    // Battery and status
    tft.setTextSize(1);
    tft.setTextColor(COLOR_INFO, COLOR_BG);
    tft.setCursor(5, 110);
    tft.printf("Bat: %.2fV", batteryVoltage);
    
    // Screen indicator
    tft.setCursor(100, 110);
    tft.printf("1/%d", maxScreens);
}

void displaySensorScreen() {
    tft.fillScreen(COLOR_BG);
    tft.setTextColor(COLOR_TEXT, COLOR_BG);
    
    // Title
    tft.setTextSize(1);
    tft.setCursor(10, 5);
    tft.setTextColor(COLOR_ACCENT, COLOR_BG);
    tft.println("SENSOR DATA");
    
    // Temperature and pressure
    tft.setTextColor(COLOR_TEXT, COLOR_BG);
    tft.setCursor(5, 25);
    if (sensorInitialized) {
        tft.printf("Temp: %.1f C", temperature);
        tft.setCursor(5, 40);
        tft.printf("Pres: %.1f hPa", pressure / 100.0);
    } else {
        tft.println("BMP180: NOT FOUND");
    }
    
    // IMU data
    tft.setCursor(5, 60);
    if (imuInitialized) {
        tft.println("IMU: ACTIVE");
        tft.setCursor(5, 75);
        tft.printf("Pitch: %.1f", pitch);
        tft.setCursor(5, 90);
        tft.printf("Roll:  %.1f", roll);
    } else {
        tft.println("IMU: NOT FOUND");
    }
    
    // Screen indicator
    tft.setCursor(100, 110);
    tft.printf("2/%d", maxScreens);
}

void displaySettingsScreen() {
    tft.fillScreen(COLOR_BG);
    tft.setTextColor(COLOR_TEXT, COLOR_BG);
    
    // Title
    tft.setTextSize(1);
    tft.setCursor(10, 5);
    tft.setTextColor(COLOR_ACCENT, COLOR_BG);
    tft.println("SETTINGS");
    
    // WiFi status
    tft.setTextColor(COLOR_TEXT, COLOR_BG);
    tft.setCursor(5, 25);
    tft.println("WiFi: OFF");
    
    // Logging status
    tft.setCursor(5, 40);
    tft.println("Logging: ON");
    
    // Memory usage
    tft.setCursor(5, 55);
    tft.printf("Free heap: %d", ESP.getFreeHeap());
    
    // Uptime
    tft.setCursor(5, 70);
    tft.printf("Uptime: %lu s", millis() / 1000);
    
    // Controls
    tft.setCursor(5, 90);
    tft.println("BTN1: Reset Max");
    tft.setCursor(5, 100);
    tft.println("BTN2: Next Screen");
    
    // Screen indicator
    tft.setCursor(100, 110);
    tft.printf("3/%d", maxScreens);
}

void handleButtons() {
    unsigned long currentTime = millis();
    
    // Button 1: Reset max altitude
    if (digitalRead(BUTTON_1_PIN) == LOW) {
        if (currentTime - lastButtonPressTime > debounceDelay) {
            maxAltitude = currentAltitude;
            Serial.println("Max altitude reset");
            setRGBLED(255, 0, 0); // Red flash
            delay(100);
            setRGBLED(0, 0, 0);
            lastButtonPressTime = currentTime;
        }
    }
    
    // Button 2: Change screen
    if (digitalRead(BUTTON_2_PIN) == LOW) {
        if (currentTime - lastButtonPressTime > debounceDelay) {
            currentScreen = (currentScreen + 1) % maxScreens;
            Serial.printf("Screen changed to %d\n", currentScreen);
            setRGBLED(0, 0, 255); // Blue flash
            delay(100);
            setRGBLED(0, 0, 0);
            lastButtonPressTime = currentTime;
        }
    }
    
    // Button 3: Toggle logging (placeholder)
    if (digitalRead(BUTTON_3_PIN) == LOW) {
        if (currentTime - lastButtonPressTime > debounceDelay) {
            Serial.println("Logging toggle (not implemented)");
            setRGBLED(0, 255, 0); // Green flash
            delay(100);
            setRGBLED(0, 0, 0);
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

void setRGBLED(uint8_t r, uint8_t g, uint8_t b) {
    // Simple RGB LED control (this is a placeholder - actual implementation depends on LED type)
    // For now, just use the data pin for basic on/off
    if (r > 0 || g > 0 || b > 0) {
        digitalWrite(RGB_LED_DATA_PIN, HIGH);
    } else {
        digitalWrite(RGB_LED_DATA_PIN, LOW);
    }
}

float readBatteryVoltage() {
    int adcValue = analogRead(BATTERY_PIN);
    float voltage = (adcValue / 4095.0) * 3.3 * 2.0; // Assuming voltage divider
    return voltage;
}