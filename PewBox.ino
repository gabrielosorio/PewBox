/*********************************************************************
This is a library for our Monochrome OLEDs based on SSD1305 drivers

  Pick one up today in the adafruit shop!
  ------> https://www.adafruit.com/products/2675

These displays use SPI or I2C to communicate, 3-5 pins are required to
interface

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.
BSD license, check license.txt for more information
All text above, and the splash screen below must be included in any redistribution
*********************************************************************/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1305.h>

// Rotary Encoder
#define ENCODER_INPUT_CLK 2
#define ENCODER_INPUT_DT 3
#define ENCODER_INPUT_SW 4
int encoderValue = 0;
int encoderValueMin = 0; // Don't go negative
int encoderValueMax = 2; // Max 3 items
int encoderCurrentCLK;
int encoderPreviousCLK;
int encoderCurrentSW;
int encoderPreviousSW;
bool encoderToggled = false;

// Used for software SPI
#define OLED_CLK 13
#define OLED_MOSI 11

// Used for software or hardware SPI
#define OLED_CS 10
//#define OLED_DC 8
#define OLED_DC 9

// Used for I2C or SPI
//#define OLED_RESET 9
#define OLED_RESET 8

// software SPI
//Adafruit_SSD1305 display(128, 64, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
// hardware SPI - use 7Mhz (7000000UL) or lower because the screen is rated for 4MHz, or it will remain blank!
Adafruit_SSD1305 display(128, 64, &SPI, OLED_DC, OLED_RESET, OLED_CS, 1000000UL);

// I2C
//Adafruit_SSD1305 display(128, 64, &Wire, OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2


#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16
static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };

void setup() {
  Serial.begin(9600);
  // Make sure serial is online before proceeding
  // while (! Serial) delay(100);
  Serial.println("SSD1305 OLED test");

  if (!display.begin(0x3C)) {
     Serial.println("Unable to initialize OLED");
     while (1) yield();
  }

  // Init Rotary Encoder
  pinMode(ENCODER_INPUT_CLK, INPUT);
  pinMode(ENCODER_INPUT_DT, INPUT);
  pinMode(ENCODER_INPUT_SW, INPUT);
  encoderPreviousCLK = digitalRead(ENCODER_INPUT_CLK); // Initial CLK state
  encoderPreviousSW = digitalRead(ENCODER_INPUT_SW); // Initial SW state

  // Init Done
  display.display(); // show splashscreen
  delay(1000);
  display.clearDisplay();   // clears the screen and buffer

  // Display Boot Text
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("PewBox");
  display.setTextSize(1);
  display.setTextColor(BLACK, WHITE); // 'inverted' text
  display.println("v0.2");
  display.display();
  delay(1000);
  display.clearDisplay();
}

uint8_t activeMenuItemIndex = 0;

void loop() {
  readEncoder();

  // Read selected meenu item from encoder
  activeMenuItemIndex = encoderValue;
  renderMenu();

  display.display();
  display.clearDisplay();
}

void renderMenu() {
  char *menuItems[] = {
    "Foo                 ",
    "Far                 ",
    "Few                 "
  };

  display.setCursor(0, 0);
  display.setTextSize(1);

  display.setTextColor(WHITE);

  for (uint8_t i=0; i < sizeof(menuItems) / sizeof(menuItems[0]); i++) {
    // Highlight item if selected
    highlightSelectedMenuItem(i);

    display.println(menuItems[i]);
  }
}

void highlightSelectedMenuItem(uint8_t renderingMenuItemIndex) {
  // If we're rendering the currently selected menu item, highlight it
  if (renderingMenuItemIndex == activeMenuItemIndex) {
    display.setTextColor(BLACK, WHITE); // 'inverted' text
  } else {
    display.setTextColor(WHITE);
  }
}

void readEncoder() {
  encoderCurrentSW = digitalRead(ENCODER_INPUT_SW);
  encoderCurrentCLK = digitalRead(ENCODER_INPUT_CLK);

  // If the SW changed, then the encoder was pressed or released
  if (encoderCurrentSW != encoderPreviousSW) {
    if (encoderCurrentSW == LOW) {
      Serial.println("Encoder Pressed");
      encoderToggled = !encoderToggled;
    }

    if (encoderCurrentSW == HIGH) {
      Serial.println("Encoder Released");
    }

    encoderPreviousSW = encoderCurrentSW;
  }

  // If the CLK changed, then the encoder moved
  if (encoderCurrentCLK != encoderPreviousCLK) {
    // Both CLK and DT are HIGH when rotating counterclockwise
    if (encoderCurrentCLK == digitalRead(ENCODER_INPUT_DT)) { // Counterclockwise
      if (encoderValue > encoderValueMin) {
        encoderValue--;
      }
    } else { // Clockwise
      if (encoderValue < encoderValueMax) {
        encoderValue++;
      }
    }

    Serial.print("Encoder Value: ");
    Serial.println(encoderValue);

    encoderPreviousCLK = encoderCurrentCLK;
  }
}

void testdrawbitmap(const uint8_t *bitmap, uint8_t w, uint8_t h) {
  uint8_t icons[NUMFLAKES][3];
  randomSeed(666);     // whatever seed

  // initialize
  for (uint8_t f=0; f<NUMFLAKES; f++) {
    icons[f][XPOS] = random(display.width());
    icons[f][YPOS] = 0;
    icons[f][DELTAY] = random(5) + 1;

    Serial.print("x: ");
    Serial.print(icons[f][XPOS], DEC);
    Serial.print(" y: ");
    Serial.print(icons[f][YPOS], DEC);
    Serial.print(" dy: ");
    Serial.println(icons[f][DELTAY], DEC);
  }

  while (1) {
    // draw each icon
    for (uint8_t f=0; f<NUMFLAKES; f++) {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS], logo16_glcd_bmp, w, h, WHITE);
    }
    display.display();
    delay(20);

    // then erase it + move it
    for (uint8_t f=0; f<NUMFLAKES; f++) {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS],  logo16_glcd_bmp, w, h, BLACK);
      // move it
      icons[f][YPOS] += icons[f][DELTAY];
      // if its gone, reinit
      if (icons[f][YPOS] > display.height()) {
	icons[f][XPOS] = random(display.width());
	icons[f][YPOS] = 0;
	icons[f][DELTAY] = random(5) + 1;
      }
    }
   }
}


void testdrawchar(void) {
  display.setTextSize(1);
  display.setTextWrap(false);
  display.setTextColor(WHITE);
  display.setCursor(0,0);

  for (uint8_t i=0; i<168; i++) {
    if (i == '\n') continue;
    display.write(i);
    if ((i > 0) && (i % 21 == 0))
      display.println();
  }
  display.display();
}

void testdrawcircle(void) {
  for (uint8_t i=0; i<display.height(); i+=2) {
    display.drawCircle(display.width()/2, display.height()/2, i, WHITE);
    display.display();
  }
}

void testfillrect(void) {
  uint8_t color = 1;
  for (uint8_t i=0; i<display.height()/2; i+=3) {
    // alternate colors
    display.fillRect(i, i, display.width()-i*2, display.height()-i*2, color%2);
    display.display();
    color++;
  }
}

void testdrawtriangle(void) {
  for (uint16_t i=0; i<min(display.width(),display.height())/2; i+=5) {
    display.drawTriangle(display.width()/2, display.height()/2-i,
                     display.width()/2-i, display.height()/2+i,
                     display.width()/2+i, display.height()/2+i, WHITE);
    display.display();
  }
}

void testfilltriangle(void) {
  uint8_t color = WHITE;
  for (int16_t i=min(display.width(),display.height())/2; i>0; i-=5) {
    display.fillTriangle(display.width()/2, display.height()/2-i,
                     display.width()/2-i, display.height()/2+i,
                     display.width()/2+i, display.height()/2+i, WHITE);
    if (color == WHITE) color = BLACK;
    else color = WHITE;
    display.display();
  }
}

void testdrawroundrect(void) {
  for (uint8_t i=0; i<display.height()/3-2; i+=2) {
    display.drawRoundRect(i, i, display.width()-2*i, display.height()-2*i, display.height()/4, WHITE);
    display.display();
  }
}

void testfillroundrect(void) {
  uint8_t color = WHITE;
  for (uint8_t i=0; i<display.height()/3-2; i+=2) {
    display.fillRoundRect(i, i, display.width()-2*i, display.height()-2*i, display.height()/4, color);
    if (color == WHITE) color = BLACK;
    else color = WHITE;
    display.display();
  }
}

void testdrawrect(void) {
  for (uint8_t i=0; i<display.height()/2; i+=2) {
    display.drawRect(i, i, display.width()-2*i, display.height()-2*i, WHITE);
    display.display();
  }
}

void testdrawline() {
  for (uint8_t i=0; i<display.width(); i+=4) {
    display.drawLine(0, 0, i, display.height()-1, WHITE);
    display.display();
  }
  for (uint8_t i=0; i<display.height(); i+=4) {
    display.drawLine(0, 0, display.width()-1, i, WHITE);
    display.display();
  }
  delay(250);

  display.clearDisplay();
  for (uint8_t i=0; i<display.width(); i+=4) {
    display.drawLine(0, display.height()-1, i, 0, WHITE);
    display.display();
  }
  for (int8_t i=display.height()-1; i>=0; i-=4) {
    display.drawLine(0, display.height()-1, display.width()-1, i, WHITE);
    display.display();
  }
  delay(250);

  display.clearDisplay();
  for (int8_t i=display.width()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, i, 0, WHITE);
    display.display();
  }
  for (int8_t i=display.height()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, 0, i, WHITE);
    display.display();
  }
  delay(250);

  display.clearDisplay();
  for (uint8_t i=0; i<display.height(); i+=4) {
    display.drawLine(display.width()-1, 0, 0, i, WHITE);
    display.display();
  }
  for (uint8_t i=0; i<display.width(); i+=4) {
    display.drawLine(display.width()-1, 0, i, display.height()-1, WHITE);
    display.display();
  }
  delay(250);
}
