#include <TFT_eSPI.h> // Include the TFT_eSPI library

TFT_eSPI tft = TFT_eSPI(); // Create an instance

// Define modern custom colors using HEX
#define BG_COLOR      0x1A1A1A  // Dark charcoal
#define TEXT_COLOR    0x00CED1  // Dark turquoise / teal
#define BORDER_COLOR  0xFFD700  // Gold for subtle accent

void setup() {
  tft.init();
  tft.setRotation(0);              
  tft.fillScreen(BG_COLOR);        

  // Optional subtle border
  tft.drawRect(5, 5, tft.width() - 10, tft.height() - 10, BORDER_COLOR);

  // Text settings
  tft.setTextColor(TEXT_COLOR, BG_COLOR);
  tft.setTextSize(2);              // Smaller, sleek text
  tft.setTextDatum(MC_DATUM);      // Center alignment

  // Dynamically center the text
  int centerX = tft.width() / 2;
  int centerY = tft.height() / 2;

  tft.drawString("Hello Bhavi", centerX, centerY); 
}

void loop() {
  // Static display, nothing to loop
}
