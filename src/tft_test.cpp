#include "tft_test.h"
#include <SPI.h>
#include <Arduino.h>

// ST7789 Commands (from CircuitPython driver)
#define ST7789_SWRESET  0x01
#define ST7789_SLPIN    0x10
#define ST7789_SLPOUT   0x11
#define ST7789_NORON    0x13
#define ST7789_INVOFF   0x20
#define ST7789_INVON    0x21
#define ST7789_DISPOFF  0x28
#define ST7789_DISPON   0x29
#define ST7789_CASET    0x2A
#define ST7789_RASET    0x2B
#define ST7789_RAMWR    0x2C
#define ST7789_COLMOD   0x3A
#define ST7789_MADCTL   0x36

// MADCTL bits
#define ST7789_MADCTL_MY  0x80
#define ST7789_MADCTL_MX  0x40
#define ST7789_MADCTL_MV  0x20
#define ST7789_MADCTL_ML  0x10
#define ST7789_MADCTL_BGR 0x08
#define ST7789_MADCTL_MH  0x04
#define ST7789_MADCTL_RGB 0x00

// Color definitions (RGB565)
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

// Pin definitions from User_Setup.h
#define TFT_CS   35
#define TFT_DC   36
#define TFT_RST  34
#define TFT_BL   33
#define TFT_MOSI 38
#define TFT_SCLK 40
#define TFT_MISO 39

// Display dimensions
#define TFT_WIDTH  128
#define TFT_HEIGHT 128

// ST7789 initialization sequence (from CircuitPython driver)
struct InitCmd {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t len;
    uint16_t delay_ms;
};

static const InitCmd init_cmds[] = {
    {0x11, {}, 0, 120},                                                    // Exit sleep mode
    {0x13, {}, 0, 0},                                                      // Turn on the display
    {0xB6, {0x0A, 0x82}, 2, 0},                                          // Set display function control
    {0x3A, {0x55}, 1, 10},                                               // Set pixel format to 16 bits per pixel (RGB565)
    {0xB2, {0x0C, 0x0C, 0x00, 0x33, 0x33}, 5, 0},                      // Set porch control
    {0xB7, {0x35}, 1, 0},                                                // Set gate control
    {0xBB, {0x28}, 1, 0},                                                // Set VCOMS setting
    {0xC0, {0x0C}, 1, 0},                                                // Set power control 1
    {0xC2, {0x01, 0xFF}, 2, 0},                                          // Set power control 2
    {0xC3, {0x10}, 1, 0},                                                // Set power control 3
    {0xC4, {0x20}, 1, 0},                                                // Set power control 4
    {0xC6, {0x0F}, 1, 0},                                                // Set VCOM control 1
    {0xD0, {0xA4, 0xA1}, 2, 0},                                          // Set power control A
    {0xE0, {0xD0, 0x00, 0x02, 0x07, 0x0A, 0x28, 0x32, 0x44, 0x42, 0x06, 0x0E, 0x12, 0x14, 0x17}, 14, 0}, // Set gamma curve positive polarity
    {0xE1, {0xD0, 0x00, 0x02, 0x07, 0x0A, 0x28, 0x31, 0x54, 0x47, 0x0E, 0x1C, 0x17, 0x1B, 0x1E}, 14, 0}, // Set gamma curve negative polarity
    {0x21, {}, 0, 0},                                                     // Enable display inversion
    {0x29, {}, 0, 120}                                                    // Turn on the display
};

TFTTest::TFTTest() {
    width = TFT_WIDTH;
    height = TFT_HEIGHT;
    rotation = 0;
}

void TFTTest::begin() {
    Serial.println("TFT Test Driver: Initializing...");
    
    // Initialize pins
    pinMode(TFT_CS, OUTPUT);
    pinMode(TFT_DC, OUTPUT);
    pinMode(TFT_RST, OUTPUT);
    pinMode(TFT_BL, OUTPUT);
    
    // Set initial states
    digitalWrite(TFT_CS, HIGH);
    digitalWrite(TFT_DC, HIGH);
    digitalWrite(TFT_RST, HIGH);
    digitalWrite(TFT_BL, LOW);  // Backlight off initially
    
    // Initialize SPI
    SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);
    SPI.setFrequency(40000000);  // 40MHz
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);
    
    Serial.println("TFT Test Driver: Hardware reset...");
    hardReset();
    
    Serial.println("TFT Test Driver: Initializing display...");
    initDisplay();
    
    Serial.println("TFT Test Driver: Setting rotation...");
    setRotation(0);
    
    Serial.println("TFT Test Driver: Turning on backlight...");
    digitalWrite(TFT_BL, HIGH);  // Turn on backlight
    
    Serial.println("TFT Test Driver: Initialization complete!");
}

void TFTTest::hardReset() {
    digitalWrite(TFT_CS, LOW);
    digitalWrite(TFT_RST, HIGH);
    delay(10);
    digitalWrite(TFT_RST, LOW);
    delay(10);
    digitalWrite(TFT_RST, HIGH);
    delay(120);
    digitalWrite(TFT_CS, HIGH);
}

void TFTTest::writeCommand(uint8_t cmd) {
    digitalWrite(TFT_CS, LOW);
    digitalWrite(TFT_DC, LOW);  // Command mode
    SPI.transfer(cmd);
    digitalWrite(TFT_CS, HIGH);
}

void TFTTest::writeData(uint8_t data) {
    digitalWrite(TFT_CS, LOW);
    digitalWrite(TFT_DC, HIGH);  // Data mode
    SPI.transfer(data);
    digitalWrite(TFT_CS, HIGH);
}

void TFTTest::writeData16(uint16_t data) {
    digitalWrite(TFT_CS, LOW);
    digitalWrite(TFT_DC, HIGH);  // Data mode
    SPI.transfer(data >> 8);
    SPI.transfer(data & 0xFF);
    digitalWrite(TFT_CS, HIGH);
}

void TFTTest::writeCommand(uint8_t cmd, uint8_t* data, uint8_t len) {
    digitalWrite(TFT_CS, LOW);
    
    // Send command
    digitalWrite(TFT_DC, LOW);
    SPI.transfer(cmd);
    
    // Send data if any
    if (len > 0 && data != nullptr) {
        digitalWrite(TFT_DC, HIGH);
        for (uint8_t i = 0; i < len; i++) {
            SPI.transfer(data[i]);
        }
    }
    
    digitalWrite(TFT_CS, HIGH);
}

void TFTTest::initDisplay() {
    const uint8_t num_cmds = sizeof(init_cmds) / sizeof(InitCmd);
    
    for (uint8_t i = 0; i < num_cmds; i++) {
        writeCommand(init_cmds[i].cmd, (uint8_t*)init_cmds[i].data, init_cmds[i].len);
        if (init_cmds[i].delay_ms > 0) {
            delay(init_cmds[i].delay_ms);
        }
    }
}

void TFTTest::setRotation(uint8_t rot) {
    rotation = rot % 4;
    
    uint8_t madctl = 0;
    
    switch (rotation) {
        case 0:  // Portrait
            madctl = 0x00;
            width = TFT_WIDTH;
            height = TFT_HEIGHT;
            xstart = 2;
            ystart = 1;
            break;
        case 1:  // Landscape
            madctl = 0x60;
            width = TFT_HEIGHT;
            height = TFT_WIDTH;
            xstart = 1;
            ystart = 2;
            break;
        case 2:  // Inverted Portrait
            madctl = 0xC0;
            width = TFT_WIDTH;
            height = TFT_HEIGHT;
            xstart = 2;
            ystart = 1;
            break;
        case 3:  // Inverted Landscape
            madctl = 0xA0;
            width = TFT_HEIGHT;
            height = TFT_WIDTH;
            xstart = 1;
            ystart = 2;
            break;
    }
    
    // Set BGR color order (matching CircuitPython driver)
    madctl |= ST7789_MADCTL_BGR;
    
    writeCommand(ST7789_MADCTL, &madctl, 1);
}

void TFTTest::setWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    if (x0 <= x1 && x1 < width && y0 <= y1 && y1 < height) {
        // Column address set
        writeCommand(ST7789_CASET);
        writeData16(x0 + xstart);
        writeData16(x1 + xstart);
        
        // Row address set
        writeCommand(ST7789_RASET);
        writeData16(y0 + ystart);
        writeData16(y1 + ystart);
        
        // Memory write
        writeCommand(ST7789_RAMWR);
    }
}

void TFTTest::fillScreen(uint16_t color) {
    fillRect(0, 0, width, height, color);
}

void TFTTest::fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if (x >= width || y >= height) return;
    
    if (x + w > width) w = width - x;
    if (y + h > height) h = height - y;
    
    setWindow(x, y, x + w - 1, y + h - 1);
    
    digitalWrite(TFT_CS, LOW);
    digitalWrite(TFT_DC, HIGH);
    
    uint32_t pixels = (uint32_t)w * h;
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;
    
    for (uint32_t i = 0; i < pixels; i++) {
        SPI.transfer(hi);
        SPI.transfer(lo);
    }
    
    digitalWrite(TFT_CS, HIGH);
}

void TFTTest::drawPixel(uint16_t x, uint16_t y, uint16_t color) {
    if (x >= width || y >= height) return;
    
    setWindow(x, y, x, y);
    writeData16(color);
}

void TFTTest::drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
    bool steep = abs(y1 - y0) > abs(x1 - x0);
    
    if (steep) {
        // Swap x and y coordinates
        uint16_t temp = x0; x0 = y0; y0 = temp;
        temp = x1; x1 = y1; y1 = temp;
    }
    
    if (x0 > x1) {
        // Swap start and end points
        uint16_t temp = x0; x0 = x1; x1 = temp;
        temp = y0; y0 = y1; y1 = temp;
    }
    
    int16_t dx = x1 - x0;
    int16_t dy = abs(y1 - y0);
    int16_t err = dx / 2;
    int16_t ystep = (y0 < y1) ? 1 : -1;
    
    for (; x0 <= x1; x0++) {
        if (steep) {
            drawPixel(y0, x0, color);
        } else {
            drawPixel(x0, y0, color);
        }
        
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

void TFTTest::drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    drawLine(x, y, x + w - 1, y, color);           // Top
    drawLine(x, y, x, y + h - 1, color);           // Left
    drawLine(x + w - 1, y, x + w - 1, y + h - 1, color); // Right
    drawLine(x, y + h - 1, x + w - 1, y + h - 1, color); // Bottom
}

void TFTTest::drawCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;
    
    drawPixel(x0, y0 + r, color);
    drawPixel(x0, y0 - r, color);
    drawPixel(x0 + r, y0, color);
    drawPixel(x0 - r, y0, color);
    
    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        
        drawPixel(x0 + x, y0 + y, color);
        drawPixel(x0 - x, y0 + y, color);
        drawPixel(x0 + x, y0 - y, color);
        drawPixel(x0 - x, y0 - y, color);
        drawPixel(x0 + y, y0 + x, color);
        drawPixel(x0 - y, y0 + x, color);
        drawPixel(x0 + y, y0 - x, color);
        drawPixel(x0 - y, y0 - x, color);
    }
}

// Color conversion function (RGB888 to RGB565)
uint16_t TFTTest::color565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// Test functions
void TFTTest::runBasicTest() {
    Serial.println("TFT Test: Running basic display test...");
    
    // Test 1: Fill screen with different colors
    Serial.println("Test 1: Color fill test");
    fillScreen(RED);
    delay(1000);
    fillScreen(GREEN);
    delay(1000);
    fillScreen(BLUE);
    delay(1000);
    fillScreen(BLACK);
    delay(500);
    
    // Test 2: Draw rectangles
    Serial.println("Test 2: Rectangle test");
    drawRect(10, 10, 50, 30, WHITE);
    drawRect(20, 20, 50, 30, YELLOW);
    drawRect(30, 30, 50, 30, CYAN);
    delay(2000);
    
    fillScreen(BLACK);
    
    // Test 3: Draw circles
    Serial.println("Test 3: Circle test");
    drawCircle(64, 64, 30, RED);
    drawCircle(64, 64, 20, GREEN);
    drawCircle(64, 64, 10, BLUE);
    delay(2000);
    
    fillScreen(BLACK);
    
    // Test 4: Draw lines
    Serial.println("Test 4: Line test");
    for (int i = 0; i < 128; i += 10) {
        drawLine(0, 0, i, 127, color565(i * 2, 255 - i * 2, 128));
        drawLine(0, 127, i, 0, color565(255 - i * 2, i * 2, 128));
    }
    delay(2000);
    
    fillScreen(BLACK);
    
    Serial.println("TFT Test: Basic test complete!");
}

void TFTTest::runRotationTest() {
    Serial.println("TFT Test: Running rotation test...");
    
    for (int rot = 0; rot < 4; rot++) {
        Serial.printf("Testing rotation %d\n", rot);
        setRotation(rot);
        
        fillScreen(BLACK);
        
        // Draw orientation indicators
        fillRect(0, 0, 20, 20, RED);           // Top-left corner
        fillRect(width-20, 0, 20, 20, GREEN);  // Top-right corner
        fillRect(0, height-20, 20, 20, BLUE); // Bottom-left corner
        fillRect(width-20, height-20, 20, 20, WHITE); // Bottom-right corner
        
        // Draw center cross
        drawLine(width/2 - 10, height/2, width/2 + 10, height/2, YELLOW);
        drawLine(width/2, height/2 - 10, width/2, height/2 + 10, YELLOW);
        
        delay(2000);
    }
    
    // Reset to default rotation
    setRotation(0);
    fillScreen(BLACK);
    
    Serial.println("TFT Test: Rotation test complete!");
}

void TFTTest::runColorTest() {
    Serial.println("TFT Test: Running color test...");
    
    fillScreen(BLACK);
    
    // Draw color gradient
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            uint8_t r = (x * 255) / width;
            uint8_t g = (y * 255) / height;
            uint8_t b = ((x + y) * 255) / (width + height);
            
            drawPixel(x, y, color565(r, g, b));
        }
    }
    
    delay(3000);
    fillScreen(BLACK);
    
    Serial.println("TFT Test: Color test complete!");
}

void TFTTest::runPerformanceTest() {
    Serial.println("TFT Test: Running performance test...");
    
    unsigned long start_time = millis();
    
    // Fill screen test
    for (int i = 0; i < 10; i++) {
        fillScreen(random(0xFFFF));
    }
    
    unsigned long fill_time = millis() - start_time;
    Serial.printf("Fill screen x10: %lu ms\n", fill_time);
    
    // Pixel drawing test
    start_time = millis();
    fillScreen(BLACK);
    
    for (int i = 0; i < 1000; i++) {
        drawPixel(random(width), random(height), random(0xFFFF));
    }
    
    unsigned long pixel_time = millis() - start_time;
    Serial.printf("Draw 1000 pixels: %lu ms\n", pixel_time);
    
    // Line drawing test
    start_time = millis();
    fillScreen(BLACK);
    
    for (int i = 0; i < 100; i++) {
        drawLine(random(width), random(height), random(width), random(height), random(0xFFFF));
    }
    
    unsigned long line_time = millis() - start_time;
    Serial.printf("Draw 100 lines: %lu ms\n", line_time);
    
    fillScreen(BLACK);
    
    Serial.println("TFT Test: Performance test complete!");
}

void TFTTest::runAllTests() {
    Serial.println("TFT Test: Running all tests...");
    
    runBasicTest();
    delay(1000);
    
    runRotationTest();
    delay(1000);
    
    runColorTest();
    delay(1000);
    
    runPerformanceTest();
    
    // Final success screen
    fillScreen(GREEN);
    drawRect(10, 10, width-20, height-20, WHITE);
    drawRect(20, 20, width-40, height-40, BLACK);
    
    // Draw checkmark pattern
    for (int i = 0; i < 20; i++) {
        drawLine(40 + i, 60 + i/2, 40 + i, 60 + i/2, WHITE);
        drawLine(60 + i, 80 - i/2, 60 + i, 80 - i/2, WHITE);
    }
    
    Serial.println("TFT Test: All tests completed successfully!");
} 