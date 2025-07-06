#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <TFT_eSPI.h> // Include TFT_eSPI library
#include <SPI.h>
#include "FS.h" // For SPIFFS
#include "SPIFFS.h" // For SPIFFS

Adafruit_BMP085 bmp180;
float maxAltitude = 0.0; // Variable to store maximum altitude

TFT_eSPI tft = TFT_eSPI(); // Initialize TFT_eSPI object

const char* dataLogFile = "/datalog.csv";
const int BATTERY_PIN = 1; // ADC1_CH0, GPIO1 for battery voltage reading
// Voltage divider parameters (adjust if your shield is different)
// Common D1 Mini Battery Shield R1=220k, R2=100k (for 3.3V ADC)
// Or R1=100k, R2=27k (if shield expects 5V tolerant ADC but ESP32 ADC is 3.3V)
// Assuming a voltage divider that maps battery voltage (e.g. 0-4.2V for LiPo) to ADC range (e.g. 0-3.3V)
// Example: Shield provides Vbat/2 -> ADC_reading * 2 * (3.3/4095)
// For LOLIN S3 Mini Pro, the VBAT pin is connected to ADC1_CH0 (GPIO 1) with a 100K/100K divider.
// So, Battery Voltage = ADC Voltage * 2.
// ADC Voltage = (ADC Reading / 4095.0) * 3.3 (assuming 3.3V Vref and 12-bit ADC)
// Battery Voltage = (ADC Reading / 4095.0) * 3.3 * 2.0

const int BUTTON_PIN = 0; // GPIO0 is usually the BOOT button, accessible on many ESP32 boards
unsigned long lastButtonPressTime = 0;
const unsigned long debounceDelay = 50; // 50 ms debounce delay

// Function to initialize SPIFFS
void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    tft.println("SPIFFS Mount Failed");
    return;
  }
  Serial.println("SPIFFS mounted successfully");

  // Check if datalog file exists, if not, create it with headers
  if (!SPIFFS.exists(dataLogFile)) {
    Serial.println("Creating datalog.csv file...");
    fs::File file = SPIFFS.open(dataLogFile, FILE_WRITE);
    if (!file) {
      Serial.println("Failed to create file");
      tft.println("Log File Error");
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
    tft.setCursor(0, tft.height() - 8); // Position at bottom
    tft.print("Log Error!");
    return;
  }

  unsigned long currentTime = millis(); // Using millis() as a simple timestamp
  String dataString = String(currentTime) + "," + String(temp) + "," + String(pres) + "," + String(alt);

  if (file.println(dataString)) {
    Serial.println("Data logged: " + dataString);
  } else {
    Serial.println("Write failed");
    tft.setCursor(0, tft.height() - 8); // Position at bottom
    tft.print("Log Write Fail!");
  }
  file.close();
}

// Function to read battery voltage
float readBatteryVoltage() {
  int adcValue = analogRead(BATTERY_PIN);
  // ESP32-S3 ADC is 12-bit (0-4095). Default Vref is usually 3.3V.
  // For LOLIN S3 Mini Pro, VBAT is connected to ADC1_CH0 (GPIO1) via a 100K/100K divider.
  // So, Battery Voltage = (ADC Reading / 4095.0) * Vref * 2.0
  // We might need to calibrate Vref or use internal Vref for better accuracy.
  // For now, assume Vref = 3.3V.
  float batteryVoltage = (adcValue / 4095.0) * 3.3 * 2.0;
  return batteryVoltage;
}

void setup() {
  Serial.begin(115200);
  Serial.println("BMP180 Altimeter Test");

  // Configure ADC
  analogReadResolution(12); // Set ADC resolution to 12-bit (0-4095)
  // adcAttachPin(BATTERY_PIN); // Attach ADC pin (may not be needed for all ESP32 cores/pins with analogRead)

  // Initialize TFT display
  tft.init();
  tft.setRotation(0); // Adjust rotation as needed (0-3)

  // Initialize Button Pin
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1); // Smallest text size
  tft.setCursor(0, 0);
  tft.println("Altimeter Initializing...");
  
  // Initialize I2C communication
  Wire.begin();
  
  // Initialize BMP180 sensor
  if (!bmp180.begin()) {
    Serial.println("Could not find BMP180 sensor, check wiring!");
    tft.println("BMP180 Error!");
    while (1);
  }
  Serial.println("BMP180 sensor initialized successfully!");
  tft.println("BMP180 OK");

  // Initialize SPIFFS
  initSPIFFS();
  tft.println("SPIFFS OK"); // Assuming initSPIFFS handles its own error messages on Serial/TFT for critical failure

  delay(1000); // Show init messages
  tft.fillScreen(TFT_BLACK); // Clear screen for data
}

void loop() {
  // Read temperature and pressure
  float temperature = bmp180.readTemperature();
  float pressure = bmp180.readPressure();
  float altitude = bmp180.readAltitude();

  // Update maximum altitude
  if (altitude > maxAltitude) {
    maxAltitude = altitude;
  }

  // Log data
  logData(temperature, pressure, altitude);
  
  // Print readings
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
  
  Serial.print("Pressure: ");
  Serial.print(pressure);
  Serial.println(" Pa");
  
  Serial.print("Altitude: ");
  Serial.print(altitude);
  Serial.println(" m");

  Serial.print("Max Altitude: ");
  Serial.print(maxAltitude);
  Serial.println(" m");
  
  Serial.println("---");

  // Display data on TFT
  tft.fillScreen(TFT_BLACK); // Clear screen before drawing
  tft.setCursor(0, 0);
  tft.setTextSize(2); // Larger text for altitude

  tft.print("Alt: ");
  tft.print(altitude, 1); // 1 decimal place
  tft.println(" m");

  tft.print("Max: ");
  tft.print(maxAltitude, 1); // 1 decimal place
  tft.println(" m");

  tft.setTextSize(1); // Smaller text for other data
  tft.print("Temp: ");
  tft.print(temperature, 1);
  tft.println(" C");

  tft.print("Pres: ");
  tft.print(pressure / 100.0, 2); // Convert Pa to hPa, 2 decimal places
  tft.println(" hPa");

  // Read and display battery voltage
  float batteryVolts = readBatteryVoltage();
  tft.print("Batt: ");
  tft.print(batteryVolts, 2); // 2 decimal places
  tft.println(" V");

  Serial.print("Battery Voltage: ");
  Serial.print(batteryVolts);
  Serial.println(" V");

  // Check for button press to reset max altitude
  if (digitalRead(BUTTON_PIN) == LOW) { // Button is pressed (assuming pull-up, so LOW when pressed)
    if (millis() - lastButtonPressTime > debounceDelay) {
      Serial.println("Button pressed - Max Altitude Reset!");
      tft.setCursor(0, tft.height() - 16); // Position for message
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
      tft.print("Max Alt Reset!");
      maxAltitude = altitude; // Reset to current altitude, or use 0.0 if preferred
      lastButtonPressTime = millis();
      delay(200); // Brief delay to show message and allow button release
      tft.setTextColor(TFT_WHITE, TFT_BLACK); // Reset text color
    }
  }
  
  delay(2000); // Wait 2 seconds before next reading
}