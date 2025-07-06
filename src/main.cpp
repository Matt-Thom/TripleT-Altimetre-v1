#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_BMP085.h>
#include <Wire.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "tft_test.h"
#include "simple_font.h"

// --- PIN DEFINITIONS ---
#define BUTTON_A_PIN 0
#define BUTTON_B_PIN 47
#define BUTTON_C_PIN 48
#define RGB_DATA 8
#define RGB_POWER 7
#define TFT_BL 33

// --- WIFI CONFIGURATION ---
const char* wifi_ssid = "Altimeter-S3";
const char* wifi_password = "altimeter123";
const char* wifi_hostname = "altimeter";

// --- GLOBAL OBJECTS ---
Adafruit_NeoPixel pixels(1, RGB_DATA, NEO_GRB + NEO_KHZ800);
Adafruit_BMP085 bmp;
TFTTest tft;
AsyncWebServer server(80);

// --- ALTIMETER STATE ---
float current_altitude = 0.0;
float max_altitude = 0.0;
float baseline_pressure = 101325.0;  // Sea level pressure in Pascals
float baseline_altitude = 0.0;       // Baseline altitude offset for zeroing
float temperature = 0.0;
float pressure = 0.0;

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

// --- STATUS ---
bool bmp_available = false;
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

  // Initialize BMP180 sensor
  Serial.println("Initializing BMP180 pressure sensor...");
  bmp_available = bmp.begin();
  if (bmp_available) {
    Serial.println("✓ BMP180 sensor initialized successfully");
    
    // Take baseline pressure reading
    delay(1000); // Wait for sensor to stabilize
    float current_pressure_hpa = bmp.readPressure() / 100.0;
    baseline_pressure = 101325.0; // Standard sea level pressure
    
    Serial.printf("✓ Current pressure: %.2f hPa\n", current_pressure_hpa);
    Serial.printf("✓ Using sea level baseline: %.2f hPa\n", baseline_pressure / 100.0);
    
    pixels.setPixelColor(0, pixels.Color(0, 255, 0)); // Green - BMP OK
    pixels.show();
  } else {
    Serial.println("✗ BMP180 sensor initialization failed!");
    Serial.println("  Check connections: SDA→GPIO12, SCL→GPIO11");
    pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // Red - BMP failed
    pixels.show();
    
    tft.fillScreen(COLOR_BACKGROUND);
    drawText(10, 40, "SENSOR ERROR", COLOR_STATUS_ERROR);
    drawText(10, 60, "CHECK BMP180", COLOR_STATUS_ERROR);
    drawText(10, 80, "CONNECTIONS", COLOR_STATUS_ERROR);
    delay(3000);
  }

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
  Serial.println("  Button A (GPIO0)  - Reset max altitude to zero");
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
  
  // Button A - Reset max altitude only
  bool button_a_current = (digitalRead(BUTTON_A_PIN) == LOW);
  if (button_a_current && !button_a_pressed && (now - last_button_press) > button_debounce) {
    last_button_press = now;
    Serial.println("Button A: Resetting max altitude to zero");
    
    if (bmp_available) {
      max_altitude = 0.0;
      Serial.printf("✓ Max altitude reset to 0m\n");
      needs_full_refresh = true;
    }
    
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
    display_mode = (display_mode + 1) % 2;
    Serial.printf("Button B: Display mode %d\n", display_mode);
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
    float raw_altitude = bmp.readAltitude(baseline_pressure);
    current_altitude = raw_altitude - baseline_altitude;
    
    // Track maximum altitude
    if (current_altitude > max_altitude) {
      max_altitude = current_altitude;
    }
    
    // Serial output every 5 seconds
    static unsigned long last_serial_output = 0;
    if (millis() - last_serial_output >= 5000) {
      Serial.printf("ALT: %.2fm (MAX: %.2fm) | TEMP: %.1f°C | PRESS: %.1f hPa\n", 
                    current_altitude, max_altitude, temperature, pressure/100.0);
      last_serial_output = millis();
    }
  }
}

void updateDisplay() {
  if (needs_full_refresh) {
    tft.fillScreen(COLOR_BACKGROUND);
    needs_full_refresh = false;
  }
  
  if (display_mode == 0) {
    drawMainDisplay();
  } else {
    drawDetailedDisplay();
  }
}

void drawMainDisplay() {
  // Header
  tft.fillRect(0, 0, 128, 20, COLOR_HEADER);
  drawText(25, 5, "ALTIMETER", COLOR_TEXT);
  
  // Status indicator
  uint16_t status_color = bmp_available ? COLOR_STATUS_OK : COLOR_STATUS_ERROR;
  tft.fillRect(5, 5, 8, 8, status_color);
  
  // Current altitude - large display
  drawText(10, 30, "CURRENT", COLOR_ALTITUDE);
  drawNumber(10, 45, current_altitude, 1, COLOR_ALTITUDE);
  drawText(90, 45, "m", COLOR_ALTITUDE);
  
  // Max altitude - always visible
  drawText(10, 65, "MAXIMUM", COLOR_MAX_ALT);
  drawNumber(10, 80, max_altitude, 1, COLOR_MAX_ALT);
  drawText(90, 80, "m", COLOR_MAX_ALT);
  
  // Temperature and pressure
  drawText(10, 100, "TEMP", COLOR_TEMP);
  drawNumber(50, 100, temperature, 1, COLOR_TEMP);
  drawText(90, 100, "C", COLOR_TEMP);
  
  drawText(10, 115, "PRESS", COLOR_PRESSURE);
  drawNumber(50, 115, pressure/100.0, 0, COLOR_PRESSURE);
  drawText(90, 115, "hPa", COLOR_PRESSURE);
}

void drawDetailedDisplay() {
  // Header
  tft.fillRect(0, 0, 128, 20, COLOR_HEADER);
  drawText(30, 5, "DETAILED", COLOR_TEXT);
  
  // Status indicator
  uint16_t status_color = bmp_available ? COLOR_STATUS_OK : COLOR_STATUS_ERROR;
  tft.fillRect(5, 5, 8, 8, status_color);
  
  // Current altitude - very large
  drawText(10, 25, "ALTITUDE", COLOR_ALTITUDE);
  drawNumber(10, 40, current_altitude, 2, COLOR_ALTITUDE);
  drawText(90, 40, "m", COLOR_ALTITUDE);
  
  // Max altitude - always visible
  drawText(10, 60, "MAXIMUM", COLOR_MAX_ALT);
  drawNumber(10, 75, max_altitude, 2, COLOR_MAX_ALT);
  drawText(90, 75, "m", COLOR_MAX_ALT);
  
  // Difference from max
  float diff = current_altitude - max_altitude;
  uint16_t diff_color = (diff >= 0) ? COLOR_STATUS_OK : COLOR_STATUS_ERROR;
  drawText(10, 95, "DIFF", diff_color);
  drawNumber(10, 110, diff, 2, diff_color);
  drawText(90, 110, "m", diff_color);
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
    if (bmp_available) {
      pixels.setPixelColor(0, pixels.Color(0, led_brightness, 0)); // Green - working
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
      for (int col = 0; col < SimpleFont::CHAR_WIDTH; col++) {
        uint8_t column = char_data[col];
        for (int row = 0; row < SimpleFont::CHAR_HEIGHT; row++) {
          if (column & (1 << row)) {
            tft.drawPixel(char_x + col, y + row, color);
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
    <title>LOLIN S3 Altimeter</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background: #f0f0f0; }
        .container { max-width: 600px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; }
        h1 { color: #333; text-align: center; }
        .data { font-size: 24px; margin: 20px 0; padding: 15px; background: #f9f9f9; border-radius: 5px; }
        .altitude { color: #00aa00; font-weight: bold; }
        .max-alt { color: #aa0000; font-weight: bold; }
        .temp { color: #0000aa; }
        .pressure { color: #aa6600; }
        .status { padding: 10px; margin: 10px 0; border-radius: 5px; }
        .ok { background: #d4edda; color: #155724; }
        .error { background: #f8d7da; color: #721c24; }
        button { padding: 10px 20px; margin: 10px; font-size: 16px; cursor: pointer; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🏔️ LOLIN S3 Altimeter</h1>
        <div id="status" class="status"></div>
        <div class="data">
            <div class="altitude">Current Altitude: <span id="altitude">--</span> m</div>
            <div class="max-alt">Maximum Altitude: <span id="max-altitude">--</span> m</div>
            <div class="temp">Temperature: <span id="temperature">--</span> °C</div>
            <div class="pressure">Pressure: <span id="pressure">--</span> hPa</div>
        </div>
        <button onclick="resetAltitude()">Reset Max Altitude</button>
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
                    document.getElementById('temperature').textContent = data.temperature.toFixed(1);
                    document.getElementById('pressure').textContent = data.pressure.toFixed(1);
                    
                    const statusDiv = document.getElementById('status');
                    if (data.bmp_status) {
                        statusDiv.className = 'status ok';
                        statusDiv.textContent = '✓ BMP180 sensor working';
                    } else {
                        statusDiv.className = 'status error';
                        statusDiv.textContent = '✗ BMP180 sensor error';
                    }
                })
                .catch(error => {
                    console.error('Error:', error);
                    document.getElementById('status').className = 'status error';
                    document.getElementById('status').textContent = '✗ Connection error';
                });
        }
        
        function resetAltitude() {
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
      needs_full_refresh = true;
    }
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
  json += "\"temperature\":" + String(temperature, 2) + ",";
  json += "\"pressure\":" + String(pressure/100.0, 2) + ",";
  json += "\"bmp_status\":" + String(bmp_available ? "true" : "false");
  json += "}";
  return json;
}