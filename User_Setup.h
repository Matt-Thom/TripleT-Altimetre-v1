// -- Definitive User_Setup.h for WEMOS LOLIN S3 Mini Pro V1.0.0 --

// This setup is based on the known-correct pins from the official
// CircuitPython demo and manufacturer's specification.

// Display driver
#define ST7789_DRIVER

// Screen dimensions
#define TFT_WIDTH  128
#define TFT_HEIGHT 128

// Pin definitions from the known-correct CircuitPython demo
#define TFT_CS   35
#define TFT_DC   36
#define TFT_RST  34
#define TFT_BL   33

// SPI Bus pins
#define TFT_MOSI 38
#define TFT_SCLK 40
#define TFT_MISO 39

// Backlight control
#define TFT_BACKLIGHT_ON HIGH

// Fonts to load
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

// SPI Frequency
#define SPI_FREQUENCY  40000000
#define SPI_READ_FREQUENCY  20000000

// We are not using touch, but the library needs this defined.
#define SPI_TOUCH_FREQUENCY  2500000
#define TOUCH_CS -1 