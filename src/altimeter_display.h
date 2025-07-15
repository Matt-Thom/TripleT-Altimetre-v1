#ifndef ALTIMETER_DISPLAY_H
#define ALTIMETER_DISPLAY_H

#include "tft_test.h"
#include <Arduino.h>

class AltimeterDisplay {
private:
    TFTTest* tft;
    
    // Display layout constants
    static const int HEADER_HEIGHT = 16;      // Reduced further to give more space
    static const int STATUS_HEIGHT = 0;       // Remove status bar completely
    static const int DATA_AREA_Y = HEADER_HEIGHT + STATUS_HEIGHT;
    static const int DATA_AREA_HEIGHT = 112;  // Increased significantly (128 - 16 = 112)
    
    // Colors (RGB565)
    static const uint16_t COLOR_BACKGROUND = 0x0000;  // Black
    static const uint16_t COLOR_HEADER = 0x001F;      // Blue
    static const uint16_t COLOR_BORDER = 0x07E0;      // Green
    static const uint16_t COLOR_TEXT = 0xFFFF;        // White
    static const uint16_t COLOR_ALTITUDE = 0x07FF;    // Cyan
    static const uint16_t COLOR_MAX_ALT = 0xF800;     // Red
    static const uint16_t COLOR_TEMP = 0xFFE0;        // Yellow
    static const uint16_t COLOR_PRESSURE = 0x001F;    // Blue
    static const uint16_t COLOR_IMU = 0xF81F;         // Magenta
    static const uint16_t COLOR_STATUS_OK = 0x07E0;   // Green
    static const uint16_t COLOR_STATUS_ERROR = 0xF800; // Red
    
    // Data storage
    float current_altitude;
    float max_altitude;
    float temperature;
    float pressure;
    float accel_x, accel_y, accel_z;
    float gyro_x, gyro_y, gyro_z;
    bool bmp_status;
    bool imu_status;
    float battery_voltage;
    int battery_percentage;
    
    // Display state
    unsigned long last_update;
    bool needs_full_refresh;
    int display_mode;
    
    // Helper functions
    void drawHeader();
    void drawStatusBar();
    void drawDataArea();
    void drawAltitudeData();
    void drawEnvironmentalData();
    void drawIMUData();
    void drawGyroData();
    void drawBatterySymbol(int x, int y, int percentage);
    void drawNumber(int x, int y, float value, int decimals, uint16_t color);
    void drawText(int x, int y, const char* text, uint16_t color);
    void clearArea(int x, int y, int width, int height);
    
public:
    AltimeterDisplay(TFTTest* display);
    
    void begin();
    void update();
    void setAltitudeData(float current, float maximum);
    void setEnvironmentalData(float temp, float press);
    void setIMUData(float ax, float ay, float az, float gx, float gy, float gz);
    void setSensorStatus(bool bmp_ok, bool imu_ok);
    void setBatteryData(float voltage, int percentage);
    void resetMaxAltitude();
    void nextDisplayMode();
    void forceRefresh();
    
    // Display modes
    enum DisplayMode {
        MODE_OVERVIEW = 0,
        MODE_ALTITUDE_DETAIL = 1,
        MODE_ENVIRONMENTAL = 2,
        MODE_IMU_DETAIL = 3,
        MODE_GYRO_DETAIL = 4,
        MODE_COUNT = 5
    };
};

#endif // ALTIMETER_DISPLAY_H 