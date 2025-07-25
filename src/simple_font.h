#ifndef SIMPLE_FONT_H
#define SIMPLE_FONT_H

#include <Arduino.h>

// 5x7 pixel bitmap font scaled to 50% larger
// Each character is 7 pixels wide, 10 pixels tall when scaled
// Stored as 5 bytes per character (each byte represents a column)

class SimpleFont {
public:
    static const int CHAR_WIDTH = 10;    // 5 * 2 = 10 (2x scaled)
    static const int CHAR_HEIGHT = 14;   // 7 * 2 = 14 (2x scaled)
    static const int CHAR_SPACING = 2;   // Keep spacing proportional
    
    // Original font data - 5 bytes per character
    static const uint8_t font_data[][5];
    
    // Get font data for a character
    static const uint8_t* getCharData(char c);
    
    // Calculate text width
    static int getTextWidth(const char* text);
    
private:
    static const int FONT_START = 32;  // Space character
    static const int FONT_END = 126;   // ~ character
};

// Font bitmap data
inline const uint8_t SimpleFont::font_data[][5] = {
    // Space (32)
    {0x00, 0x00, 0x00, 0x00, 0x00},
    // ! (33)
    {0x00, 0x00, 0x5F, 0x00, 0x00},
    // " (34)
    {0x00, 0x07, 0x00, 0x07, 0x00},
    // # (35)
    {0x14, 0x7F, 0x14, 0x7F, 0x14},
    // $ (36)
    {0x24, 0x2A, 0x7F, 0x2A, 0x12},
    // % (37)
    {0x23, 0x13, 0x08, 0x64, 0x62},
    // & (38)
    {0x36, 0x49, 0x55, 0x22, 0x50},
    // ' (39)
    {0x00, 0x05, 0x03, 0x00, 0x00},
    // ( (40)
    {0x00, 0x1C, 0x22, 0x41, 0x00},
    // ) (41)
    {0x00, 0x41, 0x22, 0x1C, 0x00},
    // * (42)
    {0x08, 0x2A, 0x1C, 0x2A, 0x08},
    // + (43)
    {0x08, 0x08, 0x3E, 0x08, 0x08},
    // , (44)
    {0x00, 0x50, 0x30, 0x00, 0x00},
    // - (45)
    {0x08, 0x08, 0x08, 0x08, 0x08},
    // . (46)
    {0x00, 0x60, 0x60, 0x00, 0x00},
    // / (47)
    {0x20, 0x10, 0x08, 0x04, 0x02},
    // 0 (48)
    {0x3E, 0x51, 0x49, 0x45, 0x3E},
    // 1 (49)
    {0x00, 0x42, 0x7F, 0x40, 0x00},
    // 2 (50)
    {0x42, 0x61, 0x51, 0x49, 0x46},
    // 3 (51)
    {0x21, 0x41, 0x45, 0x4B, 0x31},
    // 4 (52)
    {0x18, 0x14, 0x12, 0x7F, 0x10},
    // 5 (53)
    {0x27, 0x45, 0x45, 0x45, 0x39},
    // 6 (54)
    {0x3C, 0x4A, 0x49, 0x49, 0x30},
    // 7 (55)
    {0x01, 0x71, 0x09, 0x05, 0x03},
    // 8 (56)
    {0x36, 0x49, 0x49, 0x49, 0x36},
    // 9 (57)
    {0x06, 0x49, 0x49, 0x29, 0x1E},
    // : (58)
    {0x00, 0x36, 0x36, 0x00, 0x00},
    // ; (59)
    {0x00, 0x56, 0x36, 0x00, 0x00},
    // < (60)
    {0x00, 0x08, 0x14, 0x22, 0x41},
    // = (61)
    {0x14, 0x14, 0x14, 0x14, 0x14},
    // > (62)
    {0x41, 0x22, 0x14, 0x08, 0x00},
    // ? (63)
    {0x02, 0x01, 0x51, 0x09, 0x06},
    // @ (64)
    {0x32, 0x49, 0x79, 0x41, 0x3E},
    // A (65)
    {0x7E, 0x11, 0x11, 0x11, 0x7E},
    // B (66)
    {0x7F, 0x49, 0x49, 0x49, 0x36},
    // C (67)
    {0x3E, 0x41, 0x41, 0x41, 0x22},
    // D (68)
    {0x7F, 0x41, 0x41, 0x22, 0x1C},
    // E (69)
    {0x7F, 0x49, 0x49, 0x49, 0x41},
    // F (70)
    {0x7F, 0x09, 0x09, 0x09, 0x01},
    // G (71)
    {0x3E, 0x41, 0x49, 0x49, 0x7A},
    // H (72)
    {0x7F, 0x08, 0x08, 0x08, 0x7F},
    // I (73)
    {0x00, 0x41, 0x7F, 0x41, 0x00},
    // J (74)
    {0x20, 0x40, 0x41, 0x3F, 0x01},
    // K (75)
    {0x7F, 0x08, 0x14, 0x22, 0x41},
    // L (76)
    {0x7F, 0x40, 0x40, 0x40, 0x40},
    // M (77)
    {0x7F, 0x02, 0x04, 0x02, 0x7F},
    // N (78)
    {0x7F, 0x04, 0x08, 0x10, 0x7F},
    // O (79)
    {0x3E, 0x41, 0x41, 0x41, 0x3E},
    // P (80)
    {0x7F, 0x09, 0x09, 0x09, 0x06},
    // Q (81)
    {0x3E, 0x41, 0x51, 0x21, 0x5E},
    // R (82)
    {0x7F, 0x09, 0x19, 0x29, 0x46},
    // S (83)
    {0x46, 0x49, 0x49, 0x49, 0x31},
    // T (84)
    {0x01, 0x01, 0x7F, 0x01, 0x01},
    // U (85)
    {0x3F, 0x40, 0x40, 0x40, 0x3F},
    // V (86)
    {0x1F, 0x20, 0x40, 0x20, 0x1F},
    // W (87)
    {0x7F, 0x20, 0x18, 0x20, 0x7F},
    // X (88)
    {0x63, 0x14, 0x08, 0x14, 0x63},
    // Y (89)
    {0x03, 0x04, 0x78, 0x04, 0x03},
    // Z (90)
    {0x61, 0x51, 0x49, 0x45, 0x43},
    // [ (91)
    {0x00, 0x00, 0x7F, 0x41, 0x41},
    // \ (92)
    {0x02, 0x04, 0x08, 0x10, 0x20},
    // ] (93)
    {0x41, 0x41, 0x7F, 0x00, 0x00},
    // ^ (94)
    {0x04, 0x02, 0x01, 0x02, 0x04},
    // _ (95)
    {0x40, 0x40, 0x40, 0x40, 0x40},
    // ` (96)
    {0x00, 0x01, 0x02, 0x04, 0x00},
    // a (97)
    {0x20, 0x54, 0x54, 0x54, 0x78},
    // b (98)
    {0x7F, 0x48, 0x44, 0x44, 0x38},
    // c (99)
    {0x38, 0x44, 0x44, 0x44, 0x20},
    // d (100)
    {0x38, 0x44, 0x44, 0x48, 0x7F},
    // e (101)
    {0x38, 0x54, 0x54, 0x54, 0x18},
    // f (102)
    {0x08, 0x7E, 0x09, 0x01, 0x02},
    // g (103)
    {0x08, 0x14, 0x54, 0x54, 0x3C},
    // h (104)
    {0x7F, 0x08, 0x04, 0x04, 0x78},
    // i (105)
    {0x00, 0x44, 0x7D, 0x40, 0x00},
    // j (106)
    {0x20, 0x40, 0x44, 0x3D, 0x00},
    // k (107)
    {0x00, 0x7F, 0x10, 0x28, 0x44},
    // l (108)
    {0x00, 0x41, 0x7F, 0x40, 0x00},
    // m (109)
    {0x7C, 0x04, 0x18, 0x04, 0x78},
    // n (110)
    {0x7C, 0x08, 0x04, 0x04, 0x78},
    // o (111)
    {0x38, 0x44, 0x44, 0x44, 0x38},
    // p (112)
    {0x7C, 0x14, 0x14, 0x14, 0x08},
    // q (113)
    {0x08, 0x14, 0x14, 0x18, 0x7C},
    // r (114)
    {0x7C, 0x08, 0x04, 0x04, 0x08},
    // s (115)
    {0x48, 0x54, 0x54, 0x54, 0x20},
    // t (116)
    {0x04, 0x3F, 0x44, 0x40, 0x20},
    // u (117)
    {0x3C, 0x40, 0x40, 0x20, 0x7C},
    // v (118)
    {0x1C, 0x20, 0x40, 0x20, 0x1C},
    // w (119)
    {0x3C, 0x40, 0x30, 0x40, 0x3C},
    // x (120)
    {0x44, 0x28, 0x10, 0x28, 0x44},
    // y (121)
    {0x0C, 0x50, 0x50, 0x50, 0x3C},
    // z (122)
    {0x44, 0x64, 0x54, 0x4C, 0x44},
    // { (123)
    {0x00, 0x08, 0x36, 0x41, 0x00},
    // | (124)
    {0x00, 0x00, 0x7F, 0x00, 0x00},
    // } (125)
    {0x00, 0x41, 0x36, 0x08, 0x00},
    // ~ (126)
    {0x08, 0x04, 0x08, 0x10, 0x08}
};

inline const uint8_t* SimpleFont::getCharData(char c) {
    if (c < FONT_START || c > FONT_END) {
        return font_data[0]; // Return space for invalid characters
    }
    return font_data[c - FONT_START];
}

inline int SimpleFont::getTextWidth(const char* text) {
    int width = 0;
    while (*text) {
        width += CHAR_WIDTH + CHAR_SPACING;
        text++;
    }
    return width - CHAR_SPACING; // Remove last spacing
}

#endif // SIMPLE_FONT_H 