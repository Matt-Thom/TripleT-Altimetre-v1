#include <Arduino.h>
#include <Wire.h>
#include <BMP180.h>

BMP180 bmp180;

void setup() {
  Serial.begin(115200);
  Serial.println("BMP180 Altimeter Test");
  
  // Initialize I2C communication
  Wire.begin();
  
  // Initialize BMP180 sensor
  if (!bmp180.begin()) {
    Serial.println("Could not find BMP180 sensor, check wiring!");
    while (1);
  }
  
  Serial.println("BMP180 sensor initialized successfully!");
}

void loop() {
  // Read temperature and pressure
  float temperature = bmp180.readTemperature();
  float pressure = bmp180.readPressure();
  float altitude = bmp180.readAltitude();
  
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
  
  Serial.println("---");
  
  delay(2000); // Wait 2 seconds before next reading
}