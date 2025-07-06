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
    
    // Draw title based on mode
    const char* title;
    switch (display_mode) {
        case MODE_OVERVIEW:
            title = "ALTIMETER";
            break;
        case MODE_ALTITUDE_DETAIL:
            title = "ALTITUDE";
            break;
        case MODE_ENVIRONMENTAL:
            title = "ENVIRONMENT";
            break;
        case MODE_IMU_DETAIL:
            title = "IMU DATA";
            break;
        default:
            title = "ALTIMETER";
            break;
    }
    
    // Simple text rendering using rectangles (since we don't have fonts)
    drawText(10, 5, title, COLOR_TEXT);
}

void AltimeterDisplay::drawStatusBar() {
    // Clear status area
    tft->fillRect(0, HEADER_HEIGHT, 128, STATUS_HEIGHT, COLOR_BACKGROUND);
    
    // Draw sensor status indicators
    // BMP180 status
    uint16_t bmp_color = bmp_status ? COLOR_STATUS_OK : COLOR_STATUS_ERROR;
    tft->fillRect(5, HEADER_HEIGHT + 2, 8, 8, bmp_color);
    drawText(15, HEADER_HEIGHT + 2, "BMP", COLOR_TEXT);
    
    // IMU status
    uint16_t imu_color = imu_status ? COLOR_STATUS_OK : COLOR_STATUS_ERROR;
    tft->fillRect(45, HEADER_HEIGHT + 2, 8, 8, imu_color);
    drawText(55, HEADER_HEIGHT + 2, "IMU", COLOR_TEXT);
    
    // Mode indicator
    char mode_text[10];
    sprintf(mode_text, "M%d", display_mode + 1);
    drawText(95, HEADER_HEIGHT + 2, mode_text, COLOR_TEXT);
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
    int y = DATA_AREA_Y + 5;
    
    if (display_mode == MODE_OVERVIEW) {
        // Compact overview mode
        
        // Current altitude - large display
        drawText(5, y, "ALT:", COLOR_ALTITUDE);
        drawNumber(35, y, current_altitude, 1, COLOR_ALTITUDE);
        drawText(90, y, "m", COLOR_ALTITUDE);
        
        y += 15;
        
        // Max altitude
        drawText(5, y, "MAX:", COLOR_MAX_ALT);
        drawNumber(35, y, max_altitude, 1, COLOR_MAX_ALT);
        drawText(90, y, "m", COLOR_MAX_ALT);
        
        y += 15;
        
        // Temperature and pressure (compact)
        drawText(5, y, "T:", COLOR_TEMP);
        drawNumber(20, y, temperature, 1, COLOR_TEMP);
        drawText(50, y, "C", COLOR_TEMP);
        
        drawText(65, y, "P:", COLOR_PRESSURE);
        drawNumber(80, y, pressure/100.0, 0, COLOR_PRESSURE);
        
        y += 15;
        
        // Altitude bar graph
        float alt_range = max(abs(current_altitude), abs(max_altitude)) + 10;
        drawBarGraph(5, y, 118, 12, current_altitude, -alt_range, alt_range, COLOR_ALTITUDE);
        
        y += 20;
        
        // IMU status (if available)
        if (imu_status) {
            // Simple acceleration magnitude indicator
            float accel_mag = sqrt(accel_x*accel_x + accel_y*accel_y + accel_z*accel_z);
            drawText(5, y, "ACC:", COLOR_IMU);
            drawNumber(35, y, accel_mag, 2, COLOR_IMU);
            drawText(90, y, "g", COLOR_IMU);
        }
        
    } else {
        // Detailed altitude mode
        
        // Current altitude - very large
        drawText(5, y, "CURRENT ALTITUDE", COLOR_ALTITUDE);
        y += 12;
        drawNumber(10, y, current_altitude, 2, COLOR_ALTITUDE);
        drawText(90, y, "meters", COLOR_ALTITUDE);
        
        y += 20;
        
        // Maximum altitude
        drawText(5, y, "MAXIMUM ALTITUDE", COLOR_MAX_ALT);
        y += 12;
        drawNumber(10, y, max_altitude, 2, COLOR_MAX_ALT);
        drawText(90, y, "meters", COLOR_MAX_ALT);
        
        y += 20;
        
        // Altitude difference
        float diff = current_altitude - max_altitude;
        uint16_t diff_color = (diff >= 0) ? COLOR_STATUS_OK : COLOR_STATUS_ERROR;
        drawText(5, y, "DIFF FROM MAX", diff_color);
        y += 12;
        drawNumber(10, y, diff, 2, diff_color);
        drawText(90, y, "meters", diff_color);
    }
}

void AltimeterDisplay::drawEnvironmentalData() {
    int y = DATA_AREA_Y + 5;
    
    // Always show current and max altitude at top
    drawText(5, y, "ALT:", COLOR_ALTITUDE);
    drawNumber(30, y, current_altitude, 1, COLOR_ALTITUDE);
    drawText(70, y, "MAX:", COLOR_MAX_ALT);
    drawNumber(95, y, max_altitude, 1, COLOR_MAX_ALT);
    y += 15;
    
    // Temperature
    drawText(5, y, "TEMPERATURE", COLOR_TEMP);
    y += 12;
    drawNumber(10, y, temperature, 2, COLOR_TEMP);
    drawText(80, y, "°C", COLOR_TEMP);
    
    y += 15;
    
    // Temperature bar graph (-10 to 40°C range)
    drawBarGraph(5, y, 118, 8, temperature, -10, 40, COLOR_TEMP);
    
    y += 15;
    
    // Pressure
    drawText(5, y, "PRESSURE", COLOR_PRESSURE);
    y += 12;
    drawNumber(10, y, pressure/100.0, 1, COLOR_PRESSURE);
    drawText(80, y, "hPa", COLOR_PRESSURE);
    
    y += 15;
    
    // Pressure bar graph (900 to 1100 hPa range)
    drawBarGraph(5, y, 118, 8, pressure/100.0, 900, 1100, COLOR_PRESSURE);
}

void AltimeterDisplay::drawIMUData() {
    int y = DATA_AREA_Y + 5;
    
    // Always show current and max altitude at top
    drawText(5, y, "ALT:", COLOR_ALTITUDE);
    drawNumber(30, y, current_altitude, 1, COLOR_ALTITUDE);
    drawText(70, y, "MAX:", COLOR_MAX_ALT);
    drawNumber(95, y, max_altitude, 1, COLOR_MAX_ALT);
    y += 15;
    
    if (imu_status) {
        // Acceleration data
        drawText(5, y, "ACCELERATION", COLOR_IMU);
        y += 10;
        
        drawText(5, y, "X:", COLOR_IMU);
        drawNumber(20, y, accel_x, 2, COLOR_IMU);
        drawText(60, y, "g", COLOR_IMU);
        y += 8;
        
        drawText(5, y, "Y:", COLOR_IMU);
        drawNumber(20, y, accel_y, 2, COLOR_IMU);
        drawText(60, y, "g", COLOR_IMU);
        y += 8;
        
        drawText(5, y, "Z:", COLOR_IMU);
        drawNumber(20, y, accel_z, 2, COLOR_IMU);
        drawText(60, y, "g", COLOR_IMU);
        y += 12;
        
        // Gyroscope data
        drawText(5, y, "GYROSCOPE", COLOR_IMU);
        y += 10;
        
        drawText(5, y, "X:", COLOR_IMU);
        drawNumber(20, y, gyro_x, 1, COLOR_IMU);
        drawText(70, y, "°/s", COLOR_IMU);
        y += 8;
        
        drawText(5, y, "Y:", COLOR_IMU);
        drawNumber(20, y, gyro_y, 1, COLOR_IMU);
        drawText(70, y, "°/s", COLOR_IMU);
        y += 8;
        
        drawText(5, y, "Z:", COLOR_IMU);
        drawNumber(20, y, gyro_z, 1, COLOR_IMU);
        drawText(70, y, "°/s", COLOR_IMU);
        
    } else {
        drawText(5, y, "IMU NOT DETECTED", COLOR_STATUS_ERROR);
        y += 15;
        drawText(5, y, "Check connections", COLOR_TEXT);
        y += 12;
        drawText(5, y, "SDA: GPIO12", COLOR_TEXT);
        y += 8;
        drawText(5, y, "SCL: GPIO11", COLOR_TEXT);
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
    // Use bitmap font for crisp, readable text
    int char_x = x;
    
    for (int i = 0; text[i] != '\0' && char_x < 128 - SimpleFont::CHAR_WIDTH; i++) {
        char c = text[i];
        const uint8_t* char_data = SimpleFont::getCharData(c);
        
        // Draw character bitmap
        for (int col = 0; col < SimpleFont::CHAR_WIDTH; col++) {
            uint8_t column = char_data[col];
            for (int row = 0; row < SimpleFont::CHAR_HEIGHT; row++) {
                if (column & (1 << row)) {
                    tft->drawPixel(char_x + col, y + row, color);
                }
            }
        }
        
        char_x += SimpleFont::CHAR_WIDTH + SimpleFont::CHAR_SPACING;
    }
}

void AltimeterDisplay::clearArea(int x, int y, int width, int height) {
    tft->fillRect(x, y, width, height, COLOR_BACKGROUND);
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