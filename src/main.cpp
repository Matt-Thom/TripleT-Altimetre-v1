#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_BMP085.h>
#include <Wire.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "tft_test.h"
#include "altimeter_display.h"
#include "imu_simulator.h"

// --- PIN DEFINITIONS ---
#define BUTTON_A_PIN 0
#define BUTTON_B_PIN 47
#define BUTTON_C_PIN 48
#define RGB_DATA 8
#define RGB_POWER 7

// --- WIFI CONFIGURATION ---
const char* wifi_ssid = "Altimeter-S3";        // Configure your desired SSID here
const char* wifi_password = "altimeter123";     // Configure your desired password here
const char* wifi_hostname = "altimeter";

// --- GLOBAL OBJECTS ---
Adafruit_NeoPixel pixels(1, RGB_DATA, NEO_GRB + NEO_KHZ800);
Adafruit_BMP085 bmp;
TFTTest tft_display;
AltimeterDisplay altimeter(&tft_display);
IMUSimulator imu;
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

// --- BUTTON HANDLING ---
unsigned long last_button_press = 0;
const unsigned long button_debounce = 200;
bool button_a_pressed = false;
bool button_b_pressed = false;
bool button_c_pressed = false;

// --- TIMING ---
unsigned long last_sensor_update = 0;
unsigned long last_display_update = 0;
unsigned long startup_time = 0;

// --- STATUS ---
bool bmp_available = false;
bool imu_available = false;
bool system_ready = false;

// --- FUNCTION PROTOTYPES ---
void handleButtons();
void updateSensors();
void updateStatusLED();
void setupWiFi();
void setupWebServer();
String getAltimeterJSON();

void setup() {
  Serial.begin(115200);
  Wire.begin(12, 11);
  delay(500);

  startup_time = millis();

  Serial.println("========================================");
  Serial.println("    LOLIN S3 Mini Pro Altimeter v2.1");
  Serial.println("========================================");
  Serial.println("Board: LOLIN S3 Mini Pro");
  Serial.println("Display: 0.85\" 128x128 TFT (ST7789)");
  Serial.println("Sensors: BMP180 + QMI8658C IMU");
  Serial.println("RGB LED: WS2812B on GPIO8, Power=GPIO7");
  Serial.println("I2C: SDA=GPIO12, SCL=GPIO11");
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
  tft_display.begin();
  
  // Ensure backlight pin is set up for display control
  pinMode(33, OUTPUT);
  digitalWrite(33, HIGH);  // Start with display on
  
  Serial.println("✓ TFT display initialized");

  // Initialize altimeter display
  Serial.println("Initializing altimeter display interface...");
  altimeter.begin();
  Serial.println("✓ Altimeter display interface ready");

  // Initialize BMP180 sensor
  Serial.println("Initializing BMP180 pressure sensor...");
  bmp_available = bmp.begin();
  if (bmp_available) {
    Serial.println("✓ BMP180 sensor initialized successfully");
    
    // Take baseline pressure reading and use standard sea level pressure
    delay(500); // Wait for sensor to stabilize
    float current_pressure_hpa = bmp.readPressure() / 100.0;
    baseline_pressure = 101325.0; // Standard sea level pressure in Pascals
    
    Serial.printf("✓ Current pressure: %.2f hPa\n", current_pressure_hpa);
    Serial.printf("✓ Using sea level baseline: %.2f hPa\n", baseline_pressure / 100.0);
    Serial.printf("✓ Estimated current altitude: %.2f m\n", 
                  44330.0 * (1.0 - pow(current_pressure_hpa / (baseline_pressure / 100.0), 0.1903)));
    
    pixels.setPixelColor(0, pixels.Color(0, 255, 0)); // Green - BMP OK
    pixels.show();
  } else {
    Serial.println("✗ BMP180 sensor initialization failed!");
    Serial.println("  Check connections: SDA→GPIO12, SCL→GPIO11");
    pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // Red - BMP failed
    pixels.show();
  }

  // Initialize IMU
  Serial.println("Initializing IMU sensor...");
  imu_available = imu.begin();
  if (imu_available) {
    Serial.println("✓ IMU sensor initialized successfully");
  } else {
    Serial.println("✗ IMU sensor not available (using simulator)");
  }

  // Update sensor status
  altimeter.setSensorStatus(bmp_available, imu_available);

  // Initialize WiFi and Web Server
  Serial.println("Initializing WiFi and Web Server...");
  setupWiFi();
  setupWebServer();
  Serial.println("✓ WiFi and Web Server ready");

  // System ready
  system_ready = true;
  Serial.println("========================================");
  Serial.println("🚀 ALTIMETER SYSTEM READY!");
  Serial.println("========================================");
  Serial.println("WiFi Network Information:");
  Serial.printf("  SSID: %s\n", wifi_ssid);
  Serial.printf("  Password: %s\n", wifi_password);
  Serial.printf("  Web Interface: http://%s\n", WiFi.softAPIP().toString().c_str());
  Serial.println("========================================");
  Serial.println("Controls:");
  Serial.println("  Button A (GPIO0)  - Reset altitude to zero");
  Serial.println("  Button B (GPIO47) - Change display mode");
  Serial.println("  Button C (GPIO48) - Toggle display on/off");
  Serial.println("========================================");

  // Final LED status
  if (bmp_available && imu_available) {
    pixels.setPixelColor(0, pixels.Color(0, 255, 255)); // Cyan - all sensors OK
  } else if (bmp_available) {
    pixels.setPixelColor(0, pixels.Color(0, 255, 0)); // Green - BMP OK only
  } else {
    pixels.setPixelColor(0, pixels.Color(255, 100, 0)); // Orange - limited functionality
  }
  pixels.show();

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
  if (display_enabled && now - last_display_update >= 100) {  // 10Hz display updates
    altimeter.update();
    last_display_update = now;
  }
  
  // Update LED breathing effect
  updateStatusLED();
  
  delay(10);  // Small delay to prevent excessive CPU usage
}

void handleButtons() {
  unsigned long now = millis();
  
  // Button A - Reset altitude to zero (without changing barometric baseline)
  bool button_a_current = (digitalRead(BUTTON_A_PIN) == LOW);
  if (button_a_current && !button_a_pressed && (now - last_button_press) > button_debounce) {
    last_button_press = now;
    Serial.println("Button A: Resetting altitude to zero");
    
    if (bmp_available) {
      // Set current barometric altitude as the new baseline offset
      baseline_altitude = bmp.readAltitude(baseline_pressure);
      max_altitude = 0.0;
      altimeter.resetMaxAltitude();
      
      Serial.printf("✓ Altitude reset to 0m (baseline offset: %.2fm)\n", baseline_altitude);
    }
    
    // Flash orange
    pixels.setPixelColor(0, pixels.Color(255, 100, 0));
    pixels.show();
    delay(100);
  }
  button_a_pressed = button_a_current;
  
  // Button B - Change display mode
  bool button_b_current = (digitalRead(BUTTON_B_PIN) == LOW);
  if (button_b_current && !button_b_pressed && (now - last_button_press) > button_debounce) {
    last_button_press = now;
    Serial.println("Button B: Changing display mode");
    
    altimeter.nextDisplayMode();
    
    // Flash blue
    pixels.setPixelColor(0, pixels.Color(0, 100, 255));
    pixels.show();
    delay(100);
  }
  button_b_pressed = button_b_current;
  
  // Button C - Toggle display on/off for power saving
  bool button_c_current = (digitalRead(BUTTON_C_PIN) == LOW);
  if (button_c_current && !button_c_pressed && (now - last_button_press) > button_debounce) {
    last_button_press = now;
    display_enabled = !display_enabled;
    
    if (display_enabled) {
      Serial.println("Button C: Display ON");
      digitalWrite(33, HIGH);  // Turn on backlight (TFT_BL pin)
      altimeter.forceRefresh();
      // Flash green
      pixels.setPixelColor(0, pixels.Color(0, 255, 0));
    } else {
      Serial.println("Button C: Display OFF");
      digitalWrite(33, LOW);   // Turn off backlight
      // Flash red
      pixels.setPixelColor(0, pixels.Color(255, 0, 0));
    }
    
    pixels.show();
    delay(100);
  }
  button_c_pressed = button_c_current;
}

void updateSensors() {
  // Update BMP180 sensor
  if (bmp_available) {
    temperature = bmp.readTemperature();
    pressure = bmp.readPressure();
    float raw_altitude = bmp.readAltitude(baseline_pressure);
    current_altitude = raw_altitude - baseline_altitude;  // Apply baseline offset
    
    // Track maximum altitude
    if (current_altitude > max_altitude) {
      max_altitude = current_altitude;
    }
    
    // Update display with new data
    altimeter.setAltitudeData(current_altitude, max_altitude);
    altimeter.setEnvironmentalData(temperature, pressure);
    
    // Serial output every 5 seconds
    static unsigned long last_serial_output = 0;
    if (millis() - last_serial_output >= 5000) {
      Serial.printf("Altitude: %.2fm (Max: %.2fm) | Temp: %.1f°C | Pressure: %.1f hPa\n", 
                    current_altitude, max_altitude, temperature, pressure/100.0);
      last_serial_output = millis();
    }
  }
  
  // Update IMU sensor
  if (imu_available) {
    imu.update();
    altimeter.setIMUData(
      imu.getAccelX(), imu.getAccelY(), imu.getAccelZ(),
      imu.getGyroX(), imu.getGyroY(), imu.getGyroZ()
    );
  }
}

void updateStatusLED() {
  // Breathing effect for status LED
  static unsigned long led_last_update = 0;
  static int led_brightness = 0;
  static int led_direction = 1;
  
  unsigned long now = millis();
  if (now - led_last_update >= 20) {  // Update every 20ms for smooth breathing
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
      // Cyan breathing - all sensors working
      pixels.setPixelColor(0, pixels.Color(0, led_brightness, led_brightness));
    } else if (bmp_available) {
      // Green breathing - BMP working
      pixels.setPixelColor(0, pixels.Color(0, led_brightness, 0));
    } else {
      // Red breathing - sensor issues
      pixels.setPixelColor(0, pixels.Color(led_brightness, 0, 0));
    }
    
    pixels.show();
    led_last_update = now;
  }
}

void setupWiFi() {
  // Set hostname
  WiFi.setHostname(wifi_hostname);
  
  // Start WiFi Access Point
  Serial.printf("Starting WiFi Access Point: %s\n", wifi_ssid);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(wifi_ssid, wifi_password);
  
  // Get AP IP address
  IPAddress ip = WiFi.softAPIP();
  Serial.printf("✓ WiFi AP started\n");
  Serial.printf("✓ SSID: %s\n", wifi_ssid);
  Serial.printf("✓ Password: %s\n", wifi_password);
  Serial.printf("✓ IP Address: %s\n", ip.toString().c_str());
  Serial.printf("✓ Web Interface: http://%s\n", ip.toString().c_str());
}

void setupWebServer() {
  // Serve main web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = R"html(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>LOLIN S3 Altimeter</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body { 
            font-family: Arial, sans-serif; 
            margin: 20px; 
            background-color: #f0f0f0;
        }
        .container { 
            max-width: 600px; 
            margin: 0 auto; 
            background: white; 
            padding: 20px; 
            border-radius: 10px; 
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        h1 { 
            color: #333; 
            text-align: center; 
        }
        .data-grid { 
            display: grid; 
            grid-template-columns: 1fr 1fr; 
            gap: 15px; 
            margin: 20px 0; 
        }
        .data-card { 
            background: #f8f9fa; 
            padding: 15px; 
            border-radius: 8px; 
            border-left: 4px solid #007bff; 
        }
        .data-label { 
            font-size: 14px; 
            color: #666; 
            margin-bottom: 5px; 
        }
        .data-value { 
            font-size: 24px; 
            font-weight: bold; 
            color: #333; 
        }
        .data-unit { 
            font-size: 16px; 
            color: #999; 
        }
        .status { 
            text-align: center; 
            margin: 20px 0; 
        }
        .status-ok { 
            color: #28a745; 
        }
        .status-error { 
            color: #dc3545; 
        }
        .refresh-btn { 
            background: #007bff; 
            color: white; 
            border: none; 
            padding: 10px 20px; 
            border-radius: 5px; 
            cursor: pointer; 
            font-size: 16px; 
            display: block; 
            margin: 20px auto; 
        }
        .refresh-btn:hover { 
            background: #0056b3; 
        }
        @media (max-width: 480px) {
            .data-grid { 
                grid-template-columns: 1fr; 
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🚀 LOLIN S3 Altimeter</h1>
        
        <div class="data-grid">
            <div class="data-card">
                <div class="data-label">Current Altitude</div>
                <div class="data-value" id="altitude">--</div>
                <div class="data-unit">meters</div>
            </div>
            
            <div class="data-card">
                <div class="data-label">Maximum Altitude</div>
                <div class="data-value" id="max-altitude">--</div>
                <div class="data-unit">meters</div>
            </div>
            
            <div class="data-card">
                <div class="data-label">Temperature</div>
                <div class="data-value" id="temperature">--</div>
                <div class="data-unit">°C</div>
            </div>
            
            <div class="data-card">
                <div class="data-label">Pressure</div>
                <div class="data-value" id="pressure">--</div>
                <div class="data-unit">hPa</div>
            </div>
        </div>
        
        <div class="status">
            <div>BMP180: <span id="bmp-status" class="status-error">Offline</span></div>
            <div>IMU: <span id="imu-status" class="status-error">Offline</span></div>
            <div>Display: <span id="display-status" class="status-ok">Online</span></div>
        </div>
        
        <button class="refresh-btn" onclick="updateData()">🔄 Refresh Data</button>
        
        <p style="text-align: center; color: #666; font-size: 12px;">
            Last updated: <span id="last-update">Never</span>
        </p>
    </div>

    <script>
        function updateData() {
            fetch('/api/data')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('altitude').textContent = data.altitude.toFixed(1);
                    document.getElementById('max-altitude').textContent = data.max_altitude.toFixed(1);
                    document.getElementById('temperature').textContent = data.temperature.toFixed(1);
                    document.getElementById('pressure').textContent = data.pressure.toFixed(1);
                    
                    document.getElementById('bmp-status').textContent = data.bmp_status ? 'Online' : 'Offline';
                    document.getElementById('bmp-status').className = data.bmp_status ? 'status-ok' : 'status-error';
                    
                    document.getElementById('imu-status').textContent = data.imu_status ? 'Online' : 'Offline';
                    document.getElementById('imu-status').className = data.imu_status ? 'status-ok' : 'status-error';
                    
                    document.getElementById('display-status').textContent = data.display_enabled ? 'On' : 'Off';
                    document.getElementById('display-status').className = data.display_enabled ? 'status-ok' : 'status-error';
                    
                    document.getElementById('last-update').textContent = new Date().toLocaleTimeString();
                })
                .catch(error => {
                    console.error('Error fetching data:', error);
                    document.getElementById('last-update').textContent = 'Error updating';
                });
        }
        
        // Auto-refresh every 2 seconds
        setInterval(updateData, 2000);
        
        // Initial data load
        updateData();
    </script>
</body>
</html>
)html";
    request->send(200, "text/html", html);
  });

  // API endpoint for JSON data
  server.on("/api/data", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", getAltimeterJSON());
  });

  // API endpoint for commands
  server.on("/api/reset", HTTP_POST, [](AsyncWebServerRequest *request){
    if (bmp_available) {
      baseline_altitude = bmp.readAltitude(baseline_pressure);
      max_altitude = 0.0;
      altimeter.resetMaxAltitude();
      Serial.println("API: Altitude reset to zero");
    }
    request->send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Altitude reset\"}");
  });

  // Start server
  server.begin();
  Serial.println("✓ Web server started on port 80");
}

String getAltimeterJSON() {
  String json = "{";
  json += "\"altitude\":" + String(current_altitude, 2) + ",";
  json += "\"max_altitude\":" + String(max_altitude, 2) + ",";
  json += "\"temperature\":" + String(temperature, 2) + ",";
  json += "\"pressure\":" + String(pressure/100.0, 2) + ",";
  json += "\"bmp_status\":" + String(bmp_available ? "true" : "false") + ",";
  json += "\"imu_status\":" + String(imu_available ? "true" : "false") + ",";
  json += "\"display_enabled\":" + String(display_enabled ? "true" : "false") + ",";
  json += "\"uptime\":" + String(millis()) + ",";
  json += "\"free_heap\":" + String(ESP.getFreeHeap());
  json += "}";
  return json;
}