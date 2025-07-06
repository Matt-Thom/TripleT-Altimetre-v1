#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_BMP085.h>
#include <Wire.h>
#include "tft_test.h"

// --- PIN DEFINITIONS ---
#define BUTTON_A_PIN 0
#define BUTTON_B_PIN 47
#define BUTTON_C_PIN 48
#define RGB_DATA 8
#define RGB_POWER 7

Adafruit_NeoPixel pixels(1, RGB_DATA, NEO_GRB + NEO_KHZ800);
Adafruit_BMP085 bmp;
TFTTest tft_test;

// Test mode variables
int test_mode = 0;
unsigned long last_button_press = 0;
const unsigned long button_debounce = 200;

void setup() {
  Serial.begin(115200);
  Wire.begin(12, 11);
  delay(100);

  Serial.println("===== LOLIN S3 Mini Pro Altimeter v2.0 - TFT TEST =====");
  Serial.println("Board: LOLIN S3 Mini Pro");
  Serial.println("Display: 0.85\" 128x128 TFT (ST7789)");
  Serial.println("RGB LED: WS2812B on GPIO8, Power=GPIO7");
  Serial.println("IMU: QMI8658C 6D MEMS");
  Serial.println("Flash: 4MB, PSRAM: 2MB");
  Serial.println("I2C: SDA=GPIO12, SCL=GPIO11 (BMP180)");
  Serial.println("TFT Display pins: Custom test driver");
  Serial.println("==========================================");

  pinMode(BUTTON_A_PIN, INPUT_PULLUP);
  pinMode(BUTTON_B_PIN, INPUT_PULLUP);
  pinMode(BUTTON_C_PIN, INPUT_PULLUP);
  Serial.println("Buttons initialized (GPIO0, GPIO47, GPIO48)");

  pinMode(RGB_POWER, OUTPUT);
  digitalWrite(RGB_POWER, HIGH);
  pixels.begin();
  Serial.println("RGB LED initialized (WS2812B on GPIO8, Power: GPIO7)");

  Serial.println("Testing RGB LED colors...");
  pixels.setPixelColor(0, pixels.Color(255, 0, 0)); pixels.show(); delay(500);
  pixels.setPixelColor(0, pixels.Color(0, 255, 0)); pixels.show(); delay(500);
  pixels.setPixelColor(0, pixels.Color(0, 0, 255)); pixels.show(); delay(500);
  pixels.setPixelColor(0, pixels.Color(0, 0, 0)); pixels.show();

  Serial.println("Initializing TFT test driver...");
  tft_test.begin();
  Serial.println("TFT test driver initialized successfully!");

  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    // Show error on TFT
    tft_test.fillScreen(0xF800);  // Red
    tft_test.drawRect(10, 10, 108, 108, 0xFFFF);  // White border
    Serial.println("BMP180 sensor error displayed on TFT");
    while (1) {}
  }
  Serial.println("BMP180 Sensor found");

  // Show initial screen
  tft_test.fillScreen(0x0000);  // Black
  tft_test.drawRect(5, 5, 118, 118, 0x07E0);  // Green border
  tft_test.fillRect(10, 10, 108, 20, 0x001F);  // Blue header
  
  Serial.println("Setup complete! Use buttons to run tests:");
  Serial.println("Button A (GPIO0): Run basic test");
  Serial.println("Button B (GPIO47): Run rotation test");
  Serial.println("Button C (GPIO48): Run all tests");
}

void loop() {
  // Check buttons
  if (digitalRead(BUTTON_A_PIN) == LOW && (millis() - last_button_press) > button_debounce) {
    last_button_press = millis();
    Serial.println("Button A pressed - Running basic test");
    pixels.setPixelColor(0, pixels.Color(255, 100, 0)); pixels.show();
    tft_test.runBasicTest();
    pixels.setPixelColor(0, pixels.Color(0, 255, 0)); pixels.show();
    delay(500);
    pixels.setPixelColor(0, pixels.Color(0, 0, 0)); pixels.show();
  }
  
  if (digitalRead(BUTTON_B_PIN) == LOW && (millis() - last_button_press) > button_debounce) {
    last_button_press = millis();
    Serial.println("Button B pressed - Running rotation test");
    pixels.setPixelColor(0, pixels.Color(0, 100, 255)); pixels.show();
    tft_test.runRotationTest();
    pixels.setPixelColor(0, pixels.Color(0, 255, 0)); pixels.show();
    delay(500);
    pixels.setPixelColor(0, pixels.Color(0, 0, 0)); pixels.show();
  }
  
  if (digitalRead(BUTTON_C_PIN) == LOW && (millis() - last_button_press) > button_debounce) {
    last_button_press = millis();
    Serial.println("Button C pressed - Running all tests");
    pixels.setPixelColor(0, pixels.Color(255, 0, 255)); pixels.show();
    tft_test.runAllTests();
    pixels.setPixelColor(0, pixels.Color(0, 255, 0)); pixels.show();
    delay(1000);
    pixels.setPixelColor(0, pixels.Color(0, 0, 0)); pixels.show();
  }

  // Display sensor data on TFT (simple version)
  static unsigned long last_update = 0;
  if (millis() - last_update > 2000) {
    last_update = millis();
    
    // Read sensor data
    float temperature = bmp.readTemperature();
    float pressure = bmp.readPressure() / 100.0F;
    float altitude = bmp.readAltitude(1013.25);
    
    // Clear display area
    tft_test.fillRect(10, 30, 108, 88, 0x0000);
    
    // Draw simple data display (using basic rectangles as "text")
    // Temperature indicator (red bars)
    int temp_bars = (int)(temperature / 5.0) + 5;  // Scale temperature
    if (temp_bars > 15) temp_bars = 15;
    for (int i = 0; i < temp_bars && i < 15; i++) {
      tft_test.fillRect(15 + i * 6, 35, 4, 8, 0xF800);  // Red bars
    }
    
    // Pressure indicator (blue bars)
    int press_bars = (int)((pressure - 900) / 10.0);  // Scale pressure
    if (press_bars > 15) press_bars = 15;
    if (press_bars < 0) press_bars = 0;
    for (int i = 0; i < press_bars && i < 15; i++) {
      tft_test.fillRect(15 + i * 6, 55, 4, 8, 0x001F);  // Blue bars
    }
    
    // Altitude indicator (green bars)
    int alt_bars = (int)(altitude / 50.0) + 5;  // Scale altitude
    if (alt_bars > 15) alt_bars = 15;
    if (alt_bars < 0) alt_bars = 0;
    for (int i = 0; i < alt_bars && i < 15; i++) {
      tft_test.fillRect(15 + i * 6, 75, 4, 8, 0x07E0);  // Green bars
    }
    
    // Draw center dot as indicator
    tft_test.fillRect(62, 95, 4, 4, 0xFFFF);
    
    // Serial output
    Serial.printf("Temp: %.1f°C, Pressure: %.1f hPa, Altitude: %.1f m\n", 
                  temperature, pressure, altitude);
  }
  
  delay(50);
} 