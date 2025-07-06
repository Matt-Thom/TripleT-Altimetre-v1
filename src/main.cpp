#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_BMP085.h>
#include <Wire.h>

// --- PIN DEFINITIONS ---
#define BUTTON_A_PIN 0
#define BUTTON_B_PIN 47
#define BUTTON_C_PIN 48
#define RGB_DATA 8
#define RGB_POWER 7

Adafruit_NeoPixel pixels(1, RGB_DATA, NEO_GRB + NEO_KHZ800);
Adafruit_BMP085 bmp;
TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);
  Wire.begin(12, 11);
  delay(100);

  Serial.println("===== LOLIN S3 Mini Pro Altimeter v2.0 =====");
  Serial.println("Board: LOLIN S3 Mini Pro");
  Serial.println("Display: 0.85\" 128x128 TFT (ST7789)");
  Serial.println("RGB LED: WS2812B on GPIO8, Power=GPIO7");
  Serial.println("IMU: QMI8658C 6D MEMS");
  Serial.println("Flash: 4MB, PSRAM: 2MB");
  Serial.println("I2C: SDA=GPIO12, SCL=GPIO11 (BMP180)");
  Serial.println("TFT Display pins are defined in User_Setup.h");
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

  Serial.println("Attempting TFT display initialization...");
  
  // Initialize TFT display with error handling
  tft.init();
  
  // Turn on backlight
  pinMode(33, OUTPUT);
  digitalWrite(33, HIGH);
  
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("TFT OK", 20, 20, 4);
  Serial.println("TFT display initialized successfully!");

  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    tft.fillScreen(TFT_RED);
    tft.drawString("BMP ERR", 20, 50, 4);
    while (1) {}
  }
   Serial.println("BMP180 Sensor found");
}

void loop() {
  tft.setCursor(0, 0, 2);
  tft.printf("Temp:  %.2f C", bmp.readTemperature());
  tft.setCursor(0, 20, 2);
  tft.printf("Pres: %.2f hPa", bmp.readPressure() / 100.0F);
  tft.setCursor(0, 40, 2);
  tft.printf("Alt:  %.2f m", bmp.readAltitude(1013.25));

  delay(1000);
}