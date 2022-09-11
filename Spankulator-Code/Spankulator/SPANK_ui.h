// display screen
#include <Greenface_ui.h>
#define PRODUCT_NAME "The Spankulator"
#define TERMINAL_WIDTH 76 // Display area under title box
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Greenface_ui ui(&display, PRODUCT_NAME);

uint16_t get_chars_per_line()
{
    int16_t x1, y1;
    uint16_t w, h;

    ui.display->getTextBounds(" ", 0, 0, &x1, &y1, &w, &h);
    return SCREEN_WIDTH / w;
    // Serial.println("Char width: " + String(w));
    // Serial.println("Chars / line: " + String(SCREEN_WIDTH / w));
}
