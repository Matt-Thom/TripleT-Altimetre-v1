#ifndef TFT_TEST_H
#define TFT_TEST_H

#include <Arduino.h>
#include <SPI.h>

class TFTTest {
private:
    uint16_t width, height;
    uint16_t xstart, ystart;
    uint8_t rotation;
    
    // Low-level SPI communication
    void writeCommand(uint8_t cmd);
    void writeData(uint8_t data);
    void writeData16(uint16_t data);
    void writeCommand(uint8_t cmd, uint8_t* data, uint8_t len);
    
    // Display initialization
    void hardReset();
    void initDisplay();
    
    // Window and coordinate management
    void setWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    
public:
    TFTTest();
    
    // Initialization
    void begin();
    
    // Display control
    void setRotation(uint8_t rot);
    
    // Basic drawing functions
    void fillScreen(uint16_t color);
    void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
    void drawPixel(uint16_t x, uint16_t y, uint16_t color);
    void drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
    void drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
    void drawCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
    
    // Utility functions
    uint16_t color565(uint8_t r, uint8_t g, uint8_t b);
    
    // Test functions
    void runBasicTest();
    void runRotationTest();
    void runColorTest();
    void runPerformanceTest();
    void runAllTests();
    
    // Getters
    uint16_t getWidth() { return width; }
    uint16_t getHeight() { return height; }
    uint8_t getRotation() { return rotation; }
};

#endif // TFT_TEST_H 