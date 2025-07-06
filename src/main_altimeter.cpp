#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_BMP085.h>
#include <Wire.h>
#include "tft_test.h"
#include "altimeter_display.h"
#include "imu_simulator.h"

// --- PIN DEFINITIONS ---
#define BUTTON_A_PIN 0
#define BUTTON_B_PIN 47
#define BUTTON_C_PIN 48
#define RGB_DATA 8
#define RGB_POWER 7

// --- GLOBAL OBJECTS ---
Adafruit_NeoPixel pixels(1, RGB_DATA, NEO_GRB + NEO_KHZ800);
Adafruit_BMP085 bmp;
TFTTest tft_display;
AltimeterDisplay altimeter(&tft_display);
IMUSimulator imu;

// --- ALTIMETER STATE ---
float current_altitude = 0.0;
float max_altitude = 0.0;
float baseline_pressure = 1013.25;  // Sea level pressure in hPa
float temperature = 0.0;
float pressure = 0.0;

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
    float current_pressure = bmp.readPressure() / 100.0;
    baseline_pressure = 1013.25; // Standard sea level pressure in hPa
    
    Serial.printf("✓ Current pressure: %.2f hPa\n", current_pressure);
    Serial.printf("✓ Using sea level baseline: %.2f hPa\n", baseline_pressure);
    Serial.printf("✓ Estimated current altitude: %.2f m\n", 
                  44330.0 * (1.0 - pow(current_pressure / baseline_pressure, 0.1903)));
    
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

  // System ready
  system_ready = true;
  Serial.println("========================================");
  Serial.println("🚀 ALTIMETER SYSTEM READY!");
  Serial.println("========================================");
  Serial.println("Controls:");
  Serial.println("  Button A (GPIO0)  - Calibrate altitude to zero");
  Serial.println("  Button B (GPIO47) - Change display mode");
  Serial.println("  Button C (GPIO48) - Force display refresh");
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
  
  // Update display
  if (now - last_display_update >= 100) {  // 10Hz display updates
    altimeter.update();
    last_display_update = now;
  }
  
  // Update LED breathing effect
  updateStatusLED();
  
  delay(10);  // Small delay to prevent excessive CPU usage
}

void handleButtons() {
  unsigned long now = millis();
  
  // Button A - Calibrate altitude to zero (current location)
  bool button_a_current = (digitalRead(BUTTON_A_PIN) == LOW);
  if (button_a_current && !button_a_pressed && (now - last_button_press) > button_debounce) {
    last_button_press = now;
    Serial.println("Button A: Calibrating altitude to zero");
    
    if (bmp_available) {
      // Set current pressure as baseline (current location = 0m)
      baseline_pressure = bmp.readPressure() / 100.0;
      current_altitude = 0.0;
      max_altitude = 0.0;
      altimeter.resetMaxAltitude();
      
      Serial.printf("✓ New baseline pressure: %.2f hPa\n", baseline_pressure);
      Serial.println("✓ Altitude calibrated to 0m");
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
  
  // Button C - Force refresh
  bool button_c_current = (digitalRead(BUTTON_C_PIN) == LOW);
  if (button_c_current && !button_c_pressed && (now - last_button_press) > button_debounce) {
    last_button_press = now;
    Serial.println("Button C: Forcing display refresh");
    
    altimeter.forceRefresh();
    
    // Flash white
    pixels.setPixelColor(0, pixels.Color(255, 255, 255));
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
    current_altitude = bmp.readAltitude(baseline_pressure);
    
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