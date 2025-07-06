#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include "FS.h"
#include "SPIFFS.h"

Adafruit_BMP085 bmp180;
float maxAltitude = 0.0;

const char* dataLogFile = "/datalog.csv";
const int BATTERY_PIN = 1;
const int BUTTON_PIN = 0;
unsigned long lastButtonPressTime = 0;
const unsigned long debounceDelay = 50;

// Function to read battery voltage
float readBatteryVoltage() {
  int adcValue = analogRead(BATTERY_PIN);
  float batteryVoltage = (adcValue / 4095.0) * 3.3 * 2.0;
  return batteryVoltage;
}

// Function to initialize SPIFFS
void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  Serial.println("SPIFFS mounted successfully");

  // Check if datalog file exists, if not, create it with headers
  if (!SPIFFS.exists(dataLogFile)) {
    Serial.println("Creating datalog.csv file...");
    fs::File file = SPIFFS.open(dataLogFile, FILE_WRITE);
    if (!file) {
      Serial.println("Failed to create file");
      return;
    }
    file.println("Timestamp,Temperature_C,Pressure_Pa,Altitude_m");
    file.close();
    Serial.println("datalog.csv created with headers.");
  } else {
    Serial.println("datalog.csv already exists.");
  }
}

// Function to log data to SPIFFS
void logData(float temp, float pres, float alt) {
  fs::File file = SPIFFS.open(dataLogFile, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }

  unsigned long currentTime = millis(); // Using millis() as a simple timestamp
  String dataString = String(currentTime) + "," + String(temp) + "," + String(pres) + "," + String(alt);

  if (file.println(dataString)) {
    Serial.println("Data logged: " + dataString);
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void setup() {
  // Initialize serial communication (USB CDC)
  Serial.begin(115200);
  delay(3000); // Wait longer for USB CDC to initialize
  Serial.println("===== BMP180 Altimeter v1.0 (Serial Only) =====");
  Serial.println("Board: LOLIN S3 Mini");
  Serial.println("Flash: 4MB");
  Serial.println("PSRAM: Disabled");
  Serial.println("Initializing...");
  Serial.flush();

  // Configure ADC
  analogReadResolution(12);

  // Initialize I2C communication with explicit pins for ESP32-S3
  Serial.println("Initializing I2C...");
  Wire.begin(8, 9); // SDA=GPIO8, SCL=GPIO9 for LOLIN S3 Mini
  Serial.println("I2C initialized with SDA=8, SCL=9");

  // Initialize BMP180 sensor
  Serial.println("Initializing BMP180 sensor...");
  if (!bmp180.begin()) {
    Serial.println("Could not find BMP180 sensor, check wiring!");
    Serial.println("Check connections:");
    Serial.println("SDA -> GPIO8");
    Serial.println("SCL -> GPIO9");
    Serial.println("VCC -> 3.3V");
    Serial.println("GND -> GND");
    while (1) {
      delay(1000);
      Serial.println("BMP180 sensor not found - check connections");
    }
  }
  Serial.println("BMP180 sensor initialized successfully!");

  // Initialize SPIFFS
  Serial.println("Initializing SPIFFS...");
  initSPIFFS();
  Serial.println("SPIFFS initialized");

  // Initialize button pin
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.println("Button initialized on GPIO0");

  Serial.println("=== Altimeter Ready ===");
  Serial.println("Press button to reset max altitude");
  Serial.println("Data will be displayed in serial monitor");
}

void loop() {
  static unsigned long lastSensorRead = 0;
  static unsigned long lastDisplayUpdate = 0;
  static unsigned long lastLogWrite = 0;
  const unsigned long sensorInterval = 1000; // Read sensor every 1 second
  const unsigned long displayInterval = 2000;  // Update display every 2 seconds
  const unsigned long logInterval = 5000;     // Log data every 5 seconds

  unsigned long currentTime = millis();

  // Read sensor data
  if (currentTime - lastSensorRead >= sensorInterval) {
    float temperature = bmp180.readTemperature();
    float pressure = bmp180.readPressure();
    float altitude = bmp180.readAltitude();
    
    // Update maximum altitude
    if (altitude > maxAltitude) {
      maxAltitude = altitude;
    }

    // Check button press for max altitude reset
    if (digitalRead(BUTTON_PIN) == LOW) {
      unsigned long currentButtonTime = millis();
      if (currentButtonTime - lastButtonPressTime > debounceDelay) {
        maxAltitude = altitude; // Reset max altitude to current altitude
        Serial.println("*** MAX ALTITUDE RESET ***");
        Serial.println("Max altitude reset to: " + String(maxAltitude) + " m");
        lastButtonPressTime = currentButtonTime;
      }
    }

    // Log data periodically
    if (currentTime - lastLogWrite >= logInterval) {
      logData(temperature, pressure, altitude);
      lastLogWrite = currentTime;
    }

    lastSensorRead = currentTime;
  }

  // Update display (serial output)
  if (currentTime - lastDisplayUpdate >= displayInterval) {
    float temperature = bmp180.readTemperature();
    float pressure = bmp180.readPressure();
    float altitude = bmp180.readAltitude();
    float batteryVoltage = readBatteryVoltage();

    // Display data via serial
    Serial.println("=== BMP180 Altimeter Data ===");
    Serial.print("Current Altitude: ");
    Serial.print(altitude, 1);
    Serial.println(" m");
    
    Serial.print("Maximum Altitude: ");
    Serial.print(maxAltitude, 1);
    Serial.println(" m");
    
    Serial.print("Temperature: ");
    Serial.print(temperature, 1);
    Serial.println(" °C");
    
    Serial.print("Pressure: ");
    Serial.print(pressure / 100.0, 1);
    Serial.println(" hPa");
    
    Serial.print("Battery Voltage: ");
    Serial.print(batteryVoltage, 2);
    Serial.println(" V");
    
    Serial.println("Press button (GPIO0) to reset max altitude");
    Serial.println("=============================");
    Serial.println();

    lastDisplayUpdate = currentTime;
  }

  delay(10); // Small delay to prevent excessive CPU usage
}