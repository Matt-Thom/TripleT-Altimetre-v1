#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_BMP085.h>
#include <Wire.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "tft_test.h"
#include "simple_font.h"
#include "imu_simulator.h"
#include "altimeter_display.h"

// --- PIN DEFINITIONS ---
#define BUTTON_A_PIN 0
#define BUTTON_B_PIN 47
#define BUTTON_C_PIN 48
#define RGB_DATA 8
#define RGB_POWER 7
#define TFT_BL 33
#define BATTERY_PIN 1  // ADC pin for battery voltage monitoring

// --- WIFI CONFIGURATION ---
const char* wifi_ssid = "Altimeter-S3";
const char* wifi_password = "altimeter123";
const char* wifi_hostname = "altimeter";

// --- GLOBAL OBJECTS ---
Adafruit_NeoPixel pixels(1, RGB_DATA, NEO_GRB + NEO_KHZ800);
Adafruit_BMP085 bmp;
IMUSimulator imu;
TFTTest tft;
AsyncWebServer server(80);
AltimeterDisplay display(&tft);  // Add display instance

// --- ALTIMETER STATE ---
float current_altitude = 0.0;
float max_altitude = 0.0;
float baseline_pressure = 101325.0;  // Sea level pressure in Pascals
float baseline_altitude = 0.0;       // Baseline altitude offset for zeroing
float temperature = 0.0;
float pressure = 0.0;

// --- BATTERY STATE ---
float battery_voltage = 0.0;
int battery_percentage = 0;

// --- ACCELERATION STATE ---
float current_acceleration = 0.0;
float max_acceleration = 0.0;
char max_acceleration_axis = 'Z';  // Track which axis had the max acceleration
float accel_x = 0.0, accel_y = 0.0, accel_z = 0.0;
float gyro_x = 0.0, gyro_y = 0.0, gyro_z = 0.0;  // Gyroscope data

// --- DISPLAY STATE ---
bool display_enabled = true;
int display_mode = 0;  // 0=Main, 1=Detailed
bool needs_full_refresh = true;

// --- COLORS ---
#define COLOR_BACKGROUND 0x0000
#define COLOR_HEADER 0x001F
#define COLOR_TEXT 0xFFFF
#define COLOR_ALTITUDE 0x07E0
#define COLOR_MAX_ALT 0xF800
#define COLOR_ACCEL 0xF81F
#define COLOR_MAX_ACCEL 0xFC00
#define COLOR_TEMP 0xFFE0
#define COLOR_PRESSURE 0x07FF
#define COLOR_STATUS_OK 0x07E0
#define COLOR_STATUS_ERROR 0xF800

// --- BUTTON HANDLING ---
unsigned long last_button_press = 0;
const unsigned long button_debounce = 200;
bool button_a_pressed = false;
bool button_b_pressed = false;
bool button_c_pressed = false;

// --- TIMING ---
unsigned long last_sensor_update = 0;
unsigned long last_display_update = 0;
unsigned long last_battery_update = 0;

// --- STATUS ---
bool bmp_available = false;
bool imu_available = false;
bool system_ready = false;

// --- FUNCTION PROTOTYPES ---
void handleButtons();
void updateSensors();
void updateDisplay();
void updateStatusLED();
void setupWiFi();
void setupWebServer();
void drawText(int x, int y, const char* text, uint16_t color);
void drawNumber(int x, int y, float value, int decimals, uint16_t color);
void drawMainDisplay();
void drawDetailedDisplay();
String getAltimeterJSON();
void updateBattery();
float readBatteryVoltage();
int calculateBatteryPercentage(float voltage);

void setup() {
  Serial.begin(115200);
  Wire.begin(12, 11);
  delay(500);

  Serial.println("========================================");
  Serial.println("    LOLIN S3 Mini Pro Altimeter v2.1");
  Serial.println("========================================");
  Serial.println("*** PURE ALTIMETER MODE - NO TESTS ***");
  Serial.println("*** THIS IS THE MAIN ALTIMETER APP ***");
  Serial.println("Board: LOLIN S3 Mini Pro");
  Serial.println("Display: 0.85\" 128x128 TFT (ST7789)");
  Serial.println("Sensor: BMP180 Pressure/Temperature");
  Serial.println("IMU: Simulated 6-DOF IMU");
  Serial.println("========================================");

  // Initialize buttons
  pinMode(BUTTON_A_PIN, INPUT_PULLUP);
  pinMode(BUTTON_B_PIN, INPUT_PULLUP);
  pinMode(BUTTON_C_PIN, INPUT_PULLUP);
  Serial.println("✓ Buttons initialized");

  // Initialize RGB LED
  pinMode(RGB_POWER, OUTPUT);
  digitalWrite(RGB_POWER, HIGH);
  pixels.begin();
  pixels.setPixelColor(0, pixels.Color(255, 255, 0)); // Yellow - initializing
  pixels.show();
  Serial.println("✓ RGB LED initialized");

  // Initialize TFT display
  Serial.println("Initializing TFT display...");
  tft.begin();
  
  // Initialize backlight control
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);  // Start with display on
  
  Serial.println("✓ TFT display initialized");

  // Initialize display
  tft.fillScreen(COLOR_BACKGROUND);
  drawText(10, 50, "INITIALIZING...", COLOR_TEXT);
  
  // Initialize the AltimeterDisplay
  display.begin();

  // Initialize BMP180 sensor
  Serial.println("Initializing BMP180 pressure sensor...");
  bmp_available = bmp.begin();
  if (bmp_available) {
    Serial.println("✓ BMP180 sensor initialized successfully");
    
    // Take initial pressure reading for reference
    delay(1000); // Wait for sensor to stabilize
    float current_pressure_pa = bmp.readPressure();
    float current_pressure_hpa = current_pressure_pa / 100.0;
    
    // Calculate absolute altitude using standard sea level pressure
    float absolute_altitude = bmp.readAltitude(101325.0);  // Use standard sea level pressure
    
    // Set baseline for relative measurements (if needed)
    baseline_pressure = current_pressure_pa;
    baseline_altitude = absolute_altitude;
    
    // Initialize current and max altitude
    current_altitude = absolute_altitude;
    max_altitude = absolute_altitude;
    
    Serial.printf("✓ Current pressure: %.2f hPa\n", current_pressure_hpa);
    Serial.printf("✓ Absolute altitude: %.2f m above sea level\n", absolute_altitude);
    Serial.printf("✓ Baseline pressure: %.2f hPa\n", baseline_pressure / 100.0);
    Serial.printf("✓ Max altitude initialized to: %.2f m\n", max_altitude);
  } else {
    Serial.println("✗ BMP180 sensor initialization failed!");
    Serial.println("  Check connections: SDA→GPIO12, SCL→GPIO11");
  }

  // Initialize IMU simulator
  Serial.println("Initializing IMU simulator...");
  imu_available = imu.begin();
  if (imu_available) {
    Serial.println("✓ IMU simulator initialized successfully");
    
    // Initialize max acceleration with a reasonable starting value
    // Since gravity is ~1g, we expect at least that much in Z-axis
    max_acceleration = 1.0;  // Start with 1g baseline
    max_acceleration_axis = 'Z';
    Serial.printf("✓ Max acceleration initialized to: %.2fg (%c axis)\n", max_acceleration, max_acceleration_axis);
  } else {
    Serial.println("✗ IMU simulator initialization failed!");
    // Still initialize max acceleration for fallback simulation
    max_acceleration = 1.0;
    max_acceleration_axis = 'Z';
  }

  // Set LED color based on sensor status
  if (bmp_available && imu_available) {
    pixels.setPixelColor(0, pixels.Color(0, 255, 0)); // Green - both OK
  } else if (bmp_available || imu_available) {
    pixels.setPixelColor(0, pixels.Color(255, 255, 0)); // Yellow - partial
  } else {
    pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // Red - both failed
    tft.fillScreen(COLOR_BACKGROUND);
    drawText(10, 40, "SENSOR ERROR", COLOR_STATUS_ERROR);
    drawText(10, 60, "CHECK SENSORS", COLOR_STATUS_ERROR);
    drawText(10, 80, "CONNECTIONS", COLOR_STATUS_ERROR);
    delay(3000);
  }
  pixels.show();

  // Initialize WiFi and Web Server
  Serial.println("Initializing WiFi and Web Server...");
  setupWiFi();
  setupWebServer();
  Serial.println("✓ WiFi and Web Server ready");

  // System ready
  system_ready = true;
  needs_full_refresh = true;
  
  Serial.println("========================================");
  Serial.println("🚀 ALTIMETER READY!");
  Serial.println("========================================");
  Serial.println("Controls:");
  Serial.println("  Button A (GPIO0)  - Reset max altitude & acceleration to zero");
  Serial.println("  Button B (GPIO47) - Toggle display mode");
  Serial.println("  Button C (GPIO48) - Toggle display on/off");
  Serial.println("========================================");

  last_sensor_update = millis();
  last_display_update = millis();
}

void loop() {
  unsigned long now = millis();
  
  // Handle button inputs
  handleButtons();
  
  // Update sensors
  if (now - last_sensor_update >= 200) {  // 5Hz sensor updates
    updateSensors();
    last_sensor_update = now;
  }
  
  // Update battery (every 5 seconds)
  if (now - last_battery_update >= 5000) {  // Every 5 seconds
    updateBattery();
    last_battery_update = now;
  }
  
  // Update display (only if enabled)
  if (display_enabled && now - last_display_update >= 500) {  // 2Hz display updates
    updateDisplay();
    last_display_update = now;
  }
  
  // Update LED breathing effect
  updateStatusLED();
  
  delay(10);
}

void handleButtons() {
  unsigned long now = millis();
  
  // Button A - Reset max altitude and acceleration
  bool button_a_current = (digitalRead(BUTTON_A_PIN) == LOW);
  if (button_a_current && !button_a_pressed && (now - last_button_press) > button_debounce) {
    last_button_press = now;
    Serial.println("Button A: Resetting max altitude and acceleration");
    
    if (bmp_available) {
      // Reset max altitude to current altitude
      max_altitude = current_altitude;
      display.resetMaxAltitude();
      Serial.printf("✓ Max altitude reset to current: %.2f m\n", max_altitude);
    }
    if (imu_available) {
      max_acceleration = 0.0;
      Serial.printf("✓ Max acceleration reset to 0g\n");
    }
    needs_full_refresh = true;
    
    // Flash orange
    pixels.setPixelColor(0, pixels.Color(255, 100, 0));
    pixels.show();
    delay(100);
  }
  button_a_pressed = button_a_current;
  
  // Button B - Toggle display mode
  bool button_b_current = (digitalRead(BUTTON_B_PIN) == LOW);
  if (button_b_current && !button_b_pressed && (now - last_button_press) > button_debounce) {
    last_button_press = now;
    display.nextDisplayMode();
    Serial.printf("Button B: Display mode switched\n");
    needs_full_refresh = true;
    
    // Flash blue
    pixels.setPixelColor(0, pixels.Color(0, 100, 255));
    pixels.show();
    delay(100);
  }
  button_b_pressed = button_b_current;
  
  // Button C - Toggle display on/off
  bool button_c_current = (digitalRead(BUTTON_C_PIN) == LOW);
  if (button_c_current && !button_c_pressed && (now - last_button_press) > button_debounce) {
    last_button_press = now;
    display_enabled = !display_enabled;
    
    if (display_enabled) {
      Serial.println("Button C: Display ON");
      digitalWrite(TFT_BL, HIGH);
      needs_full_refresh = true;
      pixels.setPixelColor(0, pixels.Color(0, 255, 0));
    } else {
      Serial.println("Button C: Display OFF");
      digitalWrite(TFT_BL, LOW);
      pixels.setPixelColor(0, pixels.Color(255, 0, 0));
    }
    
    pixels.show();
    delay(100);
  }
  button_c_pressed = button_c_current;
}

void updateSensors() {
  if (bmp_available) {
    temperature = bmp.readTemperature();
    pressure = bmp.readPressure();
    
    // Calculate absolute altitude using standard sea level pressure
    current_altitude = bmp.readAltitude(101325.0);  // Standard sea level pressure
    
    // Track maximum altitude
    if (current_altitude > max_altitude) {
      max_altitude = current_altitude;
    }
  } else {
    // Simulate altitude changes when BMP180 is not available (for testing)
    float time_sec = millis() / 1000.0;
    current_altitude = 350.0 + 5.0 * sin(time_sec * 0.1) + 2.0 * sin(time_sec * 0.3);  // Simulate around 350m elevation
    temperature = 22.0 + 3.0 * sin(time_sec * 0.05);  // Simulated temperature
    pressure = 101325.0 + 500.0 * sin(time_sec * 0.08);  // Simulated pressure
    
    // Track maximum altitude for simulated data
    if (current_altitude > max_altitude) {
      max_altitude = current_altitude;
    }
  }
  
  if (imu_available) {
    imu.update();
    accel_x = imu.getAccelX();
    accel_y = imu.getAccelY();
    accel_z = imu.getAccelZ();
    gyro_x = imu.getGyroX();
    gyro_y = imu.getGyroY();
    gyro_z = imu.getGyroZ();
  } else {
    // Simulate acceleration when IMU is not available (for testing)
    float time_sec = millis() / 1000.0;
    accel_x = 0.3 * sin(time_sec * 0.8) + 0.1 * sin(time_sec * 2.1);
    accel_y = 0.25 * cos(time_sec * 0.6) + 0.15 * cos(time_sec * 1.8);
    accel_z = 1.0 + 0.4 * sin(time_sec * 0.4) + 0.2 * sin(time_sec * 3.2);
    gyro_x = 0.05 * sin(time_sec * 0.5);
    gyro_y = 0.03 * cos(time_sec * 0.7);
    gyro_z = 0.02 * sin(time_sec * 0.9);
  }
  
  // Calculate total acceleration magnitude for display
  current_acceleration = sqrt(accel_x*accel_x + accel_y*accel_y + accel_z*accel_z);
  
  // Track maximum acceleration from any individual axis
  // Use fabs() for proper floating-point absolute values
  float abs_accel_x = fabs(accel_x);
  float abs_accel_y = fabs(accel_y);
  float abs_accel_z = fabs(accel_z);
  
  // Find the highest acceleration from any axis
  float max_current_axis = abs_accel_x;
  char current_max_axis = 'X';
  
  if (abs_accel_y > max_current_axis) {
    max_current_axis = abs_accel_y;
    current_max_axis = 'Y';
  }
  
  if (abs_accel_z > max_current_axis) {
    max_current_axis = abs_accel_z;
    current_max_axis = 'Z';
  }
  
  // Update maximum if this reading is higher
  if (max_current_axis > max_acceleration) {
    max_acceleration = max_current_axis;
    max_acceleration_axis = current_max_axis;
  }
  
  // Serial output every 5 seconds
  static unsigned long last_serial_output = 0;
  if (millis() - last_serial_output >= 5000) {
    Serial.printf("ALT: %.2fm (MAX: %.2fm) | ACC: %.2fg (MAX: %.2fg-%c) | TEMP: %.1f°C | PRESS: %.1f hPa\n", 
                  current_altitude, max_altitude, current_acceleration, max_acceleration, max_acceleration_axis, temperature, pressure/100.0);
    Serial.printf("DEBUG ALT: raw_altitude=%.2fm, current_altitude=%.2fm, max_altitude=%.2fm\n", 
                  bmp_available ? bmp.readAltitude(baseline_pressure) : current_altitude, current_altitude, max_altitude);
    Serial.printf("DEBUG ACC: X=%.2fg, Y=%.2fg, Z=%.2fg, current_mag=%.2fg, max=%.2fg-%c\n", 
                  accel_x, accel_y, accel_z, current_acceleration, max_acceleration, max_acceleration_axis);
    last_serial_output = millis();
  }
}

void updateDisplay() {
  // Update the display with current sensor data
  display.setAltitudeData(current_altitude, max_altitude);
  display.setEnvironmentalData(temperature, pressure);
  display.setIMUData(accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z);
  display.setSensorStatus(bmp_available, imu_available);
  display.setBatteryData(battery_voltage, battery_percentage);
  
  // Update the display
  display.update();
}

void drawMainDisplay() {
  // Header
  tft.fillRect(0, 0, 128, 20, COLOR_HEADER);
  drawText(25, 5, "ALTIMETER", COLOR_TEXT);
  
  // Status indicators
  uint16_t bmp_color = bmp_available ? COLOR_STATUS_OK : COLOR_STATUS_ERROR;
  uint16_t imu_color = imu_available ? COLOR_STATUS_OK : COLOR_STATUS_ERROR;
  tft.fillRect(5, 5, 6, 6, bmp_color);
  tft.fillRect(12, 5, 6, 6, imu_color);
  
  // Current altitude - large display
  drawText(10, 25, "ALT", COLOR_ALTITUDE);
  drawNumber(35, 25, current_altitude, 1, COLOR_ALTITUDE);
  drawText(80, 25, "m", COLOR_ALTITUDE);
  
  // Max altitude
  drawText(10, 40, "MAX", COLOR_MAX_ALT);
  drawNumber(35, 40, max_altitude, 1, COLOR_MAX_ALT);
  drawText(80, 40, "m", COLOR_MAX_ALT);
  
  // Current acceleration
  drawText(10, 55, "ACC", COLOR_ACCEL);
  drawNumber(35, 55, current_acceleration, 2, COLOR_ACCEL);
  drawText(80, 55, "g", COLOR_ACCEL);
  
  // Max acceleration with axis indicator
  drawText(10, 70, "MAX", COLOR_MAX_ACCEL);
  drawNumber(35, 70, max_acceleration, 2, COLOR_MAX_ACCEL);
  drawText(80, 70, "g", COLOR_MAX_ACCEL);
  
  // Show which axis had the max acceleration
  char axis_text[2] = {max_acceleration_axis, '\0'};
  drawText(95, 70, axis_text, COLOR_MAX_ACCEL);
  
  // Temperature and pressure
  drawText(10, 90, "TEMP", COLOR_TEMP);
  drawNumber(50, 90, temperature, 1, COLOR_TEMP);
  drawText(90, 90, "C", COLOR_TEMP);
  
  drawText(10, 105, "PRESS", COLOR_PRESSURE);
  drawNumber(50, 105, pressure/100.0, 0, COLOR_PRESSURE);
  drawText(90, 105, "hPa", COLOR_PRESSURE);
}

void drawDetailedDisplay() {
  // Header
  tft.fillRect(0, 0, 128, 20, COLOR_HEADER);
  drawText(30, 5, "DETAILED", COLOR_TEXT);
  
  // Status indicators
  uint16_t bmp_color = bmp_available ? COLOR_STATUS_OK : COLOR_STATUS_ERROR;
  uint16_t imu_color = imu_available ? COLOR_STATUS_OK : COLOR_STATUS_ERROR;
  tft.fillRect(5, 5, 6, 6, bmp_color);
  tft.fillRect(12, 5, 6, 6, imu_color);
  
  // Current altitude
  drawText(10, 25, "ALTITUDE", COLOR_ALTITUDE);
  drawNumber(10, 40, current_altitude, 2, COLOR_ALTITUDE);
  drawText(90, 40, "m", COLOR_ALTITUDE);
  
  // Max altitude
  drawText(10, 55, "MAX ALT", COLOR_MAX_ALT);
  drawNumber(10, 70, max_altitude, 2, COLOR_MAX_ALT);
  drawText(90, 70, "m", COLOR_MAX_ALT);
  
  // Current acceleration
  drawText(10, 85, "ACCEL", COLOR_ACCEL);
  drawNumber(10, 100, current_acceleration, 2, COLOR_ACCEL);
  drawText(90, 100, "g", COLOR_ACCEL);
  
  // Max acceleration with axis indicator
  drawText(10, 115, "MAX ACC", COLOR_MAX_ACCEL);
  drawNumber(60, 115, max_acceleration, 2, COLOR_MAX_ACCEL);
  drawText(95, 115, "g", COLOR_MAX_ACCEL);
  
  // Show which axis had the max acceleration (on next line due to space)
  char axis_label[4];
  sprintf(axis_label, "(%c)", max_acceleration_axis);
  drawText(105, 115, axis_label, COLOR_MAX_ACCEL);
}

void updateStatusLED() {
  // Breathing effect
  static unsigned long led_last_update = 0;
  static int led_brightness = 0;
  static int led_direction = 1;
  
  unsigned long now = millis();
  if (now - led_last_update >= 20) {
    led_brightness += led_direction * 5;
    
    if (led_brightness >= 255) {
      led_brightness = 255;
      led_direction = -1;
    } else if (led_brightness <= 50) {
      led_brightness = 50;
      led_direction = 1;
    }
    
    // Color based on system status
    if (bmp_available && imu_available) {
      pixels.setPixelColor(0, pixels.Color(0, led_brightness, 0)); // Green - both working
    } else if (bmp_available || imu_available) {
      pixels.setPixelColor(0, pixels.Color(led_brightness, led_brightness, 0)); // Yellow - partial
    } else {
      pixels.setPixelColor(0, pixels.Color(led_brightness, 0, 0)); // Red - error
    }
    
    pixels.show();
    led_last_update = now;
  }
}

void drawText(int x, int y, const char* text, uint16_t color) {
  int char_x = x;
  for (int i = 0; text[i] != '\0'; i++) {
    char c = text[i];
    if (c >= 32 && c <= 126) {
      const uint8_t* char_data = SimpleFont::getCharData(c);
      // Scale font by 2x - each original pixel becomes a 2x2 block
      for (int col = 0; col < 5; col++) {  // Use original font width (5)
        uint8_t column = char_data[col];
        for (int row = 0; row < 7; row++) {  // Use original font height (7)
          if (column & (1 << row)) {
            // Draw each font pixel as a 2x2 block
            tft.drawPixel(char_x + col*2, y + row*2, color);
            tft.drawPixel(char_x + col*2 + 1, y + row*2, color);
            tft.drawPixel(char_x + col*2, y + row*2 + 1, color);
            tft.drawPixel(char_x + col*2 + 1, y + row*2 + 1, color);
          }
        }
      }
    }
    char_x += SimpleFont::CHAR_WIDTH + SimpleFont::CHAR_SPACING;
  }
}

void drawNumber(int x, int y, float value, int decimals, uint16_t color) {
  char buffer[20];
  if (decimals == 0) {
    sprintf(buffer, "%.0f", value);
  } else if (decimals == 1) {
    sprintf(buffer, "%.1f", value);
  } else {
    sprintf(buffer, "%.2f", value);
  }
  drawText(x, y, buffer, color);
}

void setupWiFi() {
  WiFi.setHostname(wifi_hostname);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(wifi_ssid, wifi_password);
  
  IPAddress ip = WiFi.softAPIP();
  Serial.printf("✓ WiFi AP: %s\n", wifi_ssid);
  Serial.printf("✓ IP: %s\n", ip.toString().c_str());
}

void setupWebServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = R"html(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>TripleT Altimeter v1</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background: #f0f0f0; }
        .container { max-width: 600px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; }
        h1 { color: #333; text-align: center; }
        .data { font-size: 24px; margin: 20px 0; padding: 15px; background: #f9f9f9; border-radius: 5px; }
        .altitude { color: #00aa00; font-weight: bold; }
        .max-alt { color: #aa0000; font-weight: bold; }
        .accel { color: #aa00aa; font-weight: bold; }
        .max-accel { color: #cc6600; font-weight: bold; }
        .temp { color: #0000aa; }
        .pressure { color: #aa6600; }
        .battery { color: #006600; font-weight: bold; }
        .status { padding: 10px; margin: 10px 0; border-radius: 5px; }
        .ok { background: #d4edda; color: #155724; }
        .error { background: #f8d7da; color: #721c24; }
        button { padding: 10px 20px; margin: 10px; font-size: 16px; cursor: pointer; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🏔️ TripleT Altimeter v1</h1>
        <div id="status" class="status"></div>
        <div class="data">
            <div class="altitude">Current Altitude: <span id="altitude">--</span> m</div>
            <div class="max-alt">Maximum Altitude: <span id="max-altitude">--</span> m</div>
            <div class="accel">Current Acceleration: <span id="acceleration">--</span> g</div>
            <div class="max-accel">Maximum Acceleration: <span id="max-acceleration">--</span> g (<span id="max-acceleration-axis">-</span> axis)</div>
            <div class="temp">Temperature: <span id="temperature">--</span> °C</div>
            <div class="pressure">Pressure: <span id="pressure">--</span> hPa</div>
            <div class="battery">Battery: <span id="battery-percentage">--</span>% (<span id="battery-voltage">--</span>V)</div>
            <div class="accel">Accelerometer: X=<span id="accel-x">--</span>g, Y=<span id="accel-y">--</span>g, Z=<span id="accel-z">--</span>g</div>
            <div class="accel">Gyroscope: X=<span id="gyro-x">--</span>°/s, Y=<span id="gyro-y">--</span>°/s, Z=<span id="gyro-z">--</span>°/s</div>
        </div>
        <button onclick="resetMaxValues()">Reset Max Values</button>
        <button onclick="toggleDisplay()">Toggle Display</button>
        <button onclick="refreshData()">Refresh Data</button>
    </div>
    
    <script>
        function updateData() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('altitude').textContent = data.altitude.toFixed(1);
                    document.getElementById('max-altitude').textContent = data.max_altitude.toFixed(1);
                    document.getElementById('acceleration').textContent = data.acceleration.toFixed(2);
                    document.getElementById('max-acceleration').textContent = data.max_acceleration.toFixed(2);
                    document.getElementById('max-acceleration-axis').textContent = data.max_acceleration_axis;
                    document.getElementById('temperature').textContent = data.temperature.toFixed(1);
                    document.getElementById('pressure').textContent = data.pressure.toFixed(1);
                    document.getElementById('battery-percentage').textContent = data.battery_percentage;
                    document.getElementById('battery-voltage').textContent = data.battery_voltage.toFixed(2);
                    document.getElementById('accel-x').textContent = data.accel_x.toFixed(2);
                    document.getElementById('accel-y').textContent = data.accel_y.toFixed(2);
                    document.getElementById('accel-z').textContent = data.accel_z.toFixed(2);
                    document.getElementById('gyro-x').textContent = data.gyro_x.toFixed(2);
                    document.getElementById('gyro-y').textContent = data.gyro_y.toFixed(2);
                    document.getElementById('gyro-z').textContent = data.gyro_z.toFixed(2);
                    
                    const statusDiv = document.getElementById('status');
                    if (data.bmp_status && data.imu_status) {
                        statusDiv.className = 'status ok';
                        statusDiv.textContent = '✓ All sensors working';
                    } else if (data.bmp_status || data.imu_status) {
                        statusDiv.className = 'status ok';
                        statusDiv.textContent = '✓ Partial sensor operation';
                    } else {
                        statusDiv.className = 'status error';
                        statusDiv.textContent = '✗ Sensor errors detected';
                    }
                })
                .catch(error => {
                    console.error('Error:', error);
                    document.getElementById('status').className = 'status error';
                    document.getElementById('status').textContent = '✗ Connection error';
                });
        }
        
        function resetMaxValues() {
            fetch('/reset', {method: 'POST'})
                .then(() => setTimeout(updateData, 500));
        }
        
        function toggleDisplay() {
            fetch('/toggle', {method: 'POST'})
                .then(() => setTimeout(updateData, 500));
        }
        
        function refreshData() {
            updateData();
        }
        
        // Update data every 2 seconds
        setInterval(updateData, 2000);
        updateData(); // Initial load
    </script>
</body>
</html>
)html";
    request->send(200, "text/html", html);
  });

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", getAltimeterJSON());
  });

  server.on("/reset", HTTP_POST, [](AsyncWebServerRequest *request){
    if (bmp_available) {
      max_altitude = 0.0;
    }
    if (imu_available) {
      max_acceleration = 0.0;
    }
    needs_full_refresh = true;
    request->send(200, "text/plain", "OK");
  });

  server.on("/toggle", HTTP_POST, [](AsyncWebServerRequest *request){
    display_enabled = !display_enabled;
    digitalWrite(TFT_BL, display_enabled ? HIGH : LOW);
    if (display_enabled) needs_full_refresh = true;
    request->send(200, "text/plain", display_enabled ? "ON" : "OFF");
  });

  server.begin();
}

String getAltimeterJSON() {
  String json = "{";
  json += "\"altitude\":" + String(current_altitude, 2) + ",";
  json += "\"max_altitude\":" + String(max_altitude, 2) + ",";
  json += "\"acceleration\":" + String(current_acceleration, 2) + ",";
  json += "\"max_acceleration\":" + String(max_acceleration, 2) + ",";
  json += "\"max_acceleration_axis\":\"" + String(max_acceleration_axis) + "\",";
  json += "\"accel_x\":" + String(accel_x, 2) + ",";
  json += "\"accel_y\":" + String(accel_y, 2) + ",";
  json += "\"accel_z\":" + String(accel_z, 2) + ",";
  json += "\"gyro_x\":" + String(gyro_x, 2) + ",";
  json += "\"gyro_y\":" + String(gyro_y, 2) + ",";
  json += "\"gyro_z\":" + String(gyro_z, 2) + ",";
  json += "\"temperature\":" + String(temperature, 2) + ",";
  json += "\"pressure\":" + String(pressure/100.0, 2) + ",";
  json += "\"battery_voltage\":" + String(battery_voltage, 2) + ",";
  json += "\"battery_percentage\":" + String(battery_percentage) + ",";
  json += "\"bmp_status\":" + String(bmp_available ? "true" : "false") + ",";
  json += "\"imu_status\":" + String(imu_available ? "true" : "false");
  json += "}";
  return json;
}

void updateBattery() {
  battery_voltage = readBatteryVoltage();
  battery_percentage = calculateBatteryPercentage(battery_voltage);
}

float readBatteryVoltage() {
  // Read ADC value and convert to voltage
  // ESP32-S3 ADC resolution is 12-bit (0-4095)
  // Reference voltage is 3.3V, but we need to account for voltage divider
  int adc_reading = analogRead(BATTERY_PIN);
  
  // Convert ADC reading to voltage (assuming 3.3V reference)
  float voltage = (adc_reading * 3.3) / 4095.0;
  
  // If using a voltage divider (e.g., 2:1 ratio), multiply by the ratio
  // For direct connection, use voltage as-is
  // For 2:1 voltage divider, multiply by 2
  voltage *= 2.0;  // Assuming 2:1 voltage divider
  
  return voltage;
}

int calculateBatteryPercentage(float voltage) {
  // LiPo battery voltage ranges (typical):
  // 4.2V = 100% (fully charged)
  // 3.7V = 50% (nominal)
  // 3.0V = 0% (empty)
  
  const float min_voltage = 3.0;  // Empty battery
  const float max_voltage = 4.2;  // Full battery
  
  // Clamp voltage to valid range
  voltage = constrain(voltage, min_voltage, max_voltage);
  
  // Calculate percentage
  int percentage = (int)(((voltage - min_voltage) / (max_voltage - min_voltage)) * 100.0);
  
  return constrain(percentage, 0, 100);
}