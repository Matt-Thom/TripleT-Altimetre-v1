#include "altimeter_display.h"
#include "simple_font.h"
#include <Arduino.h>

AltimeterDisplay::AltimeterDisplay(TFTTest* display) {
    tft = display;
    current_altitude = 0.0;
    max_altitude = 0.0;
    temperature = 0.0;
    pressure = 0.0;
    accel_x = accel_y = accel_z = 0.0;
    gyro_x = gyro_y = gyro_z = 0.0;
    bmp_status = false;
    imu_status = false;
    battery_voltage = 0.0;
    battery_percentage = 0;
    last_update = 0;
    needs_full_refresh = true;
    display_mode = MODE_OVERVIEW;
}

void AltimeterDisplay::begin() {
    tft->fillScreen(COLOR_BACKGROUND);
    drawHeader();
    drawStatusBar();
    needs_full_refresh = true;
}

void AltimeterDisplay::update() {
    unsigned long now = millis();
    
    // Update display every 500ms or on mode change
    if (now - last_update > 500 || needs_full_refresh) {
        if (needs_full_refresh) {
            tft->fillScreen(COLOR_BACKGROUND);
            drawHeader();
            drawStatusBar();
            needs_full_refresh = false;
        }
        
        drawDataArea();
        last_update = now;
    }
}

void AltimeterDisplay::drawHeader() {
    // Draw header background
    tft->fillRect(0, 0, 128, HEADER_HEIGHT, COLOR_HEADER);
    
    // Draw title based on mode - use shorter titles
    const char* title;
    switch (display_mode) {
        case MODE_OVERVIEW:
            title = "ALT";  // Shortened from "ALTIMETER"
            break;
        case MODE_ALTITUDE_DETAIL:
            title = "ALT";    // Shortened from "ALTITUDE"
            break;
        case MODE_ENVIRONMENTAL:
            title = "ENV";    // Shortened from "ENVIRONMENT"
            break;
        case MODE_IMU_DETAIL:
            title = "IMU";    // Shortened from "IMU DATA"
            break;
        default:
            title = "ALT";
            break;
    }
    
    // Draw title text
    drawText(2, 3, title, COLOR_TEXT);
    
    // Draw battery symbol in top right corner of header
    drawBatterySymbol(128 - 20, 2, battery_percentage);
}

void AltimeterDisplay::drawStatusBar() {
    // Status bar removed - battery now in header, sensor status not needed on all screens
}

void AltimeterDisplay::drawDataArea() {
    // Clear data area
    clearArea(0, DATA_AREA_Y, 128, DATA_AREA_HEIGHT);
    
    switch (display_mode) {
        case MODE_OVERVIEW:
            drawAltitudeData();
            break;
        case MODE_ALTITUDE_DETAIL:
            drawAltitudeData();
            break;
        case MODE_ENVIRONMENTAL:
            drawEnvironmentalData();
            break;
        case MODE_IMU_DETAIL:
            drawIMUData();
            break;
    }
}

void AltimeterDisplay::drawAltitudeData() {
    int y = DATA_AREA_Y + 5;  // Start with small margin
    
    if (display_mode == MODE_OVERVIEW) {
        // Overview mode - clean and readable
        
        // Current altitude - large display
        drawText(2, y, "ALT:", COLOR_ALTITUDE);
        drawNumber(28, y, current_altitude, 1, COLOR_ALTITUDE);
        drawText(80, y, "m", COLOR_ALTITUDE);
        
        y += 20;  // Good spacing
        
        // Max altitude
        drawText(2, y, "MAX:", COLOR_MAX_ALT);
        drawNumber(28, y, max_altitude, 1, COLOR_MAX_ALT);
        drawText(80, y, "m", COLOR_MAX_ALT);
        
        y += 20;  // Good spacing
        
        // Temperature and pressure on separate lines for readability
        drawText(2, y, "TEMP:", COLOR_TEMP);
        drawNumber(35, y, temperature, 1, COLOR_TEMP);
        drawText(80, y, "C", COLOR_TEMP);
        
        y += 20;  // Good spacing
        
        drawText(2, y, "PRESS:", COLOR_PRESSURE);
        drawNumber(40, y, pressure/100.0, 0, COLOR_PRESSURE);
        drawText(80, y, "hPa", COLOR_PRESSURE);
        
        y += 20;  // Good spacing
        
        // IMU status (if available) - ensure it fits on screen
        if (imu_status && y < (DATA_AREA_Y + DATA_AREA_HEIGHT - 15)) {
            // Simple acceleration magnitude indicator
            float accel_mag = sqrt(accel_x*accel_x + accel_y*accel_y + accel_z*accel_z);
            drawText(2, y, "ACC:", COLOR_IMU);
            drawNumber(28, y, accel_mag, 2, COLOR_IMU);
            drawText(80, y, "g", COLOR_IMU);
        }
        
    } else {
        // Detailed altitude mode with better spacing
        
        // Current altitude
        drawText(2, y, "ALT: ", COLOR_ALTITUDE);
        y += 16;  // Space for label
        drawNumber(5, y, current_altitude, 2, COLOR_ALTITUDE);
        drawText(80, y, "m", COLOR_ALTITUDE);
        
        y += 25;  // Good spacing
        
        // Maximum altitude
        drawText(2, y, "MAX: ", COLOR_MAX_ALT);
        y += 16;  // Space for label
        drawNumber(5, y, max_altitude, 2, COLOR_MAX_ALT);
        drawText(80, y, "m", COLOR_MAX_ALT);
        
        y += 25;  // Good spacing
        
        // Altitude difference - only if there's space
        if (y < (DATA_AREA_Y + DATA_AREA_HEIGHT - 35)) {
            float diff = current_altitude - max_altitude;
            uint16_t diff_color = (diff >= 0) ? COLOR_STATUS_OK : COLOR_STATUS_ERROR;
            drawText(2, y, "DIFF: ", diff_color);
            y += 16;  // Space for label
            drawNumber(5, y, diff, 2, diff_color);
            drawText(80, y, "m", diff_color);
        }
    }
}

void AltimeterDisplay::drawEnvironmentalData() {
    int y = DATA_AREA_Y + 5;  // Start with small margin
    
    // Current and max altitude on separate lines to avoid overlap
    drawText(2, y, "ALT:", COLOR_ALTITUDE);
    drawNumber(28, y, current_altitude, 1, COLOR_ALTITUDE);
    drawText(80, y, "m", COLOR_ALTITUDE);
    
    y += 20;  // Good spacing
    
    drawText(2, y, "MAX:", COLOR_MAX_ALT);
    drawNumber(28, y, max_altitude, 1, COLOR_MAX_ALT);
    drawText(80, y, "m", COLOR_MAX_ALT);
    
    y += 25;  // Extra spacing before environmental data
    
    // Temperature
    drawText(2, y, "TEMP:", COLOR_TEMP);
    y += 16;  // Space for label
    drawNumber(5, y, temperature, 2, COLOR_TEMP);
    drawText(65, y, "°C", COLOR_TEMP);
    
    y += 25;  // Good spacing
    
    // Pressure - ensure it fits on screen
    if (y < (DATA_AREA_Y + DATA_AREA_HEIGHT - 35)) {
        drawText(2, y, "PRESS:", COLOR_PRESSURE);
        y += 16;  // Space for label
        drawNumber(5, y, pressure/100.0, 1, COLOR_PRESSURE);
        drawText(65, y, "hPa", COLOR_PRESSURE);
    }
}

void AltimeterDisplay::drawIMUData() {
    int y = DATA_AREA_Y + 5;  // Start with small margin
    
    if (imu_status) {
        // Acceleration data
        drawText(2, y, "ACCEL:", COLOR_IMU);
        y += 16;  // Space for label
        
        drawText(2, y, "X:", COLOR_IMU);
        drawNumber(16, y, accel_x, 2, COLOR_IMU);
        drawText(55, y, "g", COLOR_IMU);
        y += 18;  // Good spacing
        
        drawText(2, y, "Y:", COLOR_IMU);
        drawNumber(16, y, accel_y, 2, COLOR_IMU);
        drawText(55, y, "g", COLOR_IMU);
        y += 18;  // Good spacing
        
        drawText(2, y, "Z:", COLOR_IMU);
        drawNumber(16, y, accel_z, 2, COLOR_IMU);
        drawText(55, y, "g", COLOR_IMU);
        y += 25;  // Extra spacing before gyro
        
        // Gyroscope data - only if there's space
        if (y < (DATA_AREA_Y + DATA_AREA_HEIGHT - 70)) {
            drawText(2, y, "GYRO:", COLOR_IMU);
            y += 16;  // Space for label
            
            drawText(2, y, "X:", COLOR_IMU);
            drawNumber(16, y, gyro_x, 1, COLOR_IMU);
            drawText(55, y, "°/s", COLOR_IMU);
            y += 18;  // Good spacing
            
            drawText(2, y, "Y:", COLOR_IMU);
            drawNumber(16, y, gyro_y, 1, COLOR_IMU);
            drawText(55, y, "°/s", COLOR_IMU);
            y += 18;  // Good spacing
            
            // Only show Z if there's space
            if (y < (DATA_AREA_Y + DATA_AREA_HEIGHT - 20)) {
                drawText(2, y, "Z:", COLOR_IMU);
                drawNumber(16, y, gyro_z, 1, COLOR_IMU);
                drawText(55, y, "°/s", COLOR_IMU);
            }
        }
        
    } else {
        drawText(2, y, "IMU NOT FOUND", COLOR_STATUS_ERROR);
        y += 20;  // Good spacing
        drawText(2, y, "Check wiring", COLOR_TEXT);
        y += 18;  // Good spacing
        drawText(2, y, "SDA: GPIO12", COLOR_TEXT);
        y += 18;  // Good spacing
        drawText(2, y, "SCL: GPIO11", COLOR_TEXT);
    }
}

void AltimeterDisplay::drawBarGraph(int x, int y, int width, int height, float value, float min_val, float max_val, uint16_t color) {
    // Draw border
    tft->drawRect(x, y, width, height, COLOR_TEXT);
    
    // Calculate bar position
    float range = max_val - min_val;
    float normalized = (value - min_val) / range;
    normalized = constrain(normalized, 0.0, 1.0);
    
    int bar_width = (int)(normalized * (width - 2));
    
    // Clear interior
    tft->fillRect(x + 1, y + 1, width - 2, height - 2, COLOR_BACKGROUND);
    
    // Draw bar
    if (bar_width > 0) {
        tft->fillRect(x + 1, y + 1, bar_width, height - 2, color);
    }
    
    // Draw center line for reference
    int center_x = x + width / 2;
    tft->drawLine(center_x, y, center_x, y + height - 1, COLOR_TEXT);
}

void AltimeterDisplay::drawNumber(int x, int y, float value, int decimals, uint16_t color) {
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

void AltimeterDisplay::drawText(int x, int y, const char* text, uint16_t color) {
    // Use bitmap font for crisp, readable text with 2x scaling
    int char_x = x;
    
    for (int i = 0; text[i] != '\0' && char_x < 128 - SimpleFont::CHAR_WIDTH; i++) {
        char c = text[i];
        const uint8_t* char_data = SimpleFont::getCharData(c);
        
        // Scale font by 2x - each original pixel becomes a 2x2 block
        for (int col = 0; col < 5; col++) {  // Use original font width (5)
            uint8_t column = char_data[col];
            for (int row = 0; row < 7; row++) {  // Use original font height (7)
                if (column & (1 << row)) {
                    // Draw each font pixel as a 2x2 block
                    tft->drawPixel(char_x + col*2, y + row*2, color);
                    tft->drawPixel(char_x + col*2 + 1, y + row*2, color);
                    tft->drawPixel(char_x + col*2, y + row*2 + 1, color);
                    tft->drawPixel(char_x + col*2 + 1, y + row*2 + 1, color);
                }
            }
        }
        
        char_x += SimpleFont::CHAR_WIDTH + SimpleFont::CHAR_SPACING;
    }
}

void AltimeterDisplay::clearArea(int x, int y, int width, int height) {
    tft->fillRect(x, y, width, height, COLOR_BACKGROUND);
}

void AltimeterDisplay::drawBatterySymbol(int x, int y, int percentage) {
    // Battery symbol dimensions - very compact for header
    const int width = 12;
    const int height = 6;
    const int tip_width = 1;
    const int tip_height = 2;
    
    // Ensure percentage is valid (fix zero percent issue)
    if (percentage < 0) percentage = 0;
    if (percentage > 100) percentage = 100;
    
    // Draw battery outline
    tft->drawRect(x, y, width, height, COLOR_TEXT);
    
    // Draw battery tip
    tft->fillRect(x + width, y + (height - tip_height) / 2, tip_width, tip_height, COLOR_TEXT);
    
    // Clear interior
    tft->fillRect(x + 1, y + 1, width - 2, height - 2, COLOR_BACKGROUND);
    
    // Calculate fill level
    int fill_width = ((width - 2) * percentage) / 100;
    
    // Choose color based on battery level
    uint16_t fill_color;
    if (percentage > 50) {
        fill_color = COLOR_STATUS_OK;  // Green
    } else if (percentage > 20) {
        fill_color = COLOR_TEMP;       // Yellow
    } else {
        fill_color = COLOR_STATUS_ERROR;  // Red
    }
    
    // Draw fill
    if (fill_width > 0) {
        tft->fillRect(x + 1, y + 1, fill_width, height - 2, fill_color);
    }
    
    // No percentage text - just the icon
}

// Public methods
void AltimeterDisplay::setAltitudeData(float current, float maximum) {
    current_altitude = current;
    if (maximum > max_altitude) {
        max_altitude = maximum;
    }
}

void AltimeterDisplay::setEnvironmentalData(float temp, float press) {
    temperature = temp;
    pressure = press;
}

void AltimeterDisplay::setIMUData(float ax, float ay, float az, float gx, float gy, float gz) {
    accel_x = ax;
    accel_y = ay;
    accel_z = az;
    gyro_x = gx;
    gyro_y = gy;
    gyro_z = gz;
}

void AltimeterDisplay::setSensorStatus(bool bmp_ok, bool imu_ok) {
    bmp_status = bmp_ok;
    imu_status = imu_ok;
}

void AltimeterDisplay::setBatteryData(float voltage, int percentage) {
    battery_voltage = voltage;
    battery_percentage = percentage;
}

void AltimeterDisplay::resetMaxAltitude() {
    max_altitude = current_altitude;
}

void AltimeterDisplay::nextDisplayMode() {
    display_mode = (display_mode + 1) % MODE_COUNT;
    needs_full_refresh = true;
}

void AltimeterDisplay::forceRefresh() {
    needs_full_refresh = true;
} 