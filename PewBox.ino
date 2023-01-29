#include <SPI.h>
#include <Adafruit_SSD1305.h>

// OLED SPI
#define OLED_CS 10
#define OLED_DC 9
#define OLED_RESET 8

// hardware SPI - use 7Mhz (7000000UL) or lower because the screen is rated for 4MHz, or it will remain blank!
Adafruit_SSD1305 display(128, 64, &SPI, OLED_DC, OLED_RESET, OLED_CS, 1000000UL);

// Main Menu
#define MENU_SIZE 5 // 1-based count (one blank) - must be a constant/macro to be used to define an array

// Rotary Encoder Pins
#define ENCODER_INPUT_CLK 2
#define ENCODER_INPUT_DT 3
#define ENCODER_INPUT_SW 4

// Rotary Encoder local control values
int encoderValue = 0;
int encoderValueMin = 0; // Don't go negative
int encoderValueMax = MENU_SIZE - 1; // 0-based count, as opposed to the 1-based MENU_SIZE

#include "encoder.h"
#include "menu.h";

#define LED 6

// Teensy Audio Library
#include <Audio.h>
#include <Wire.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       filter_lfo;      //xy=391,336
AudioSynthWaveform       waveform1;      //xy=396,277
AudioFilterLadder        filter1;        //xy=574,315
AudioOutputAnalog        dac1;           //xy=752,315
AudioConnection          patchCord1(filter_lfo, 0, filter1, 1);
AudioConnection          patchCord2(waveform1, 0, filter1, 0);
AudioConnection          patchCord3(filter1, dac1);
// GUItool: end automatically generated code

void setup() {
  Serial.begin(9600);
  // Make sure serial is online before proceeding
  while (! Serial) delay(100);

  if (!display.begin(0x3C)) {
     Serial.println("Unable to initialize OLED");
     while (1) yield();
  }

  initRotaryEncoder();

  // Init Done
  display.display(); // show splashscreen
  delay(1000);
  display.clearDisplay();   // clears the screen and buffer

  displayBootScreen();

  pinMode(LED, OUTPUT);

  // Menu Setup
  // initMenu();

  // Audio Setup
  // initAudioComponents();

  // display.display();
}

uint8_t stepTicker = 0;

void loop() {
  readEncoderSwitch();
  readEncoderRotation(
    *menuControlClockwiseHandler,
    *menuControlCounterclockwiseHandler
  );

  // Read selected meenu item from encoder
  // renderMenu(encoderValue);

  // display.display();
  // display.clearDisplay();

  // Audio
  // renderAudioComponentsFromMenu();

  uint8_t gridRows = 2;
  uint8_t gridColumns = 8;

  // TODO: Review passing by reference vs by pointer
  unsigned char bitmap[gridRows] = {
    B00100101,
    B10001000
  }; // Bits are traversed back-to-front

  // Dummy Step Cycle
  drawGridFromBitmap(stepTicker, bitmap, gridRows, gridColumns);

  if (stepTicker == 15) {
    stepTicker = 0;
  } else {
    stepTicker++;
  }

  display.display();

  delay(300);

  display.clearDisplay();
}

void drawGridFromBitmap(uint8_t activeStepIndex, uint8_t *bitmap, uint8_t gridRows, uint8_t gridColumns) { // Technically bitmap or just byte array?
  uint8_t cellSize = 10;

  // TODO: Control layer (showing cursor position)
  // Only one active at a time, can infer from step number

  // TODO: Playback layer (showing active step position)
  // Only one active at a time, can infer from step number

  // Go through the rows
  for  (uint8_t row = 0; row < gridRows; row++) {
    uint8_t cellOffsetY = row * cellSize;

    // Go through the columns in the row
    for (uint8_t column = 0; column < gridColumns; column++) {
      uint8_t cellOffsetX = column * cellSize;

      byte currentBit = bitRead(bitmap[row], column); // needs to pass *bitmap if not already a pointer

      uint8_t currentStepIndex = gridColumns * row + column;
      bool isActiveStep = activeStepIndex == currentStepIndex;

      if (currentBit == 1) {
        if (isActiveStep) {
          handleCurrentStepOn();
        }
        drawMarkedCell(cellOffsetX, cellOffsetY, cellSize, cellSize, isActiveStep);
      } else {
        if (isActiveStep) {
          handleCurrentStepOff();
        }
        drawUnmarkedCell(cellOffsetX, cellOffsetY, cellSize, cellSize, isActiveStep);
      }
    }
  }
}

// Sequencer event stepping on an enabled trigger
void handleCurrentStepOn() {
  // Dummy output LED to simulate trigger
  digitalWrite(LED, HIGH);
}

// Sequencer event stepping on a disabled trigger
void handleCurrentStepOff() {
  // Dummy output LED to simulate trigger
  digitalWrite(LED, LOW);
}

void drawUnmarkedCell(uint8_t x, uint8_t y, uint8_t cellWidth, uint8_t cellHeight, bool isActiveStep) {
  // Fill rectangle if the step is currently active
  if (isActiveStep) {
    // Draw filled rectangle
    display.fillRect(x, y, cellWidth, cellHeight, WHITE);
  }

  // Draw rectangle border
  display.drawRect(x, y, cellWidth, cellHeight, isActiveStep ? BLACK : WHITE);
}

void drawMarkedCell(uint8_t x, uint8_t y, uint8_t cellWidth, uint8_t cellHeight, bool isActiveStep) {
  // Fill rectangle if the step is currently active
  if (isActiveStep) {
    // Draw filled rectangle
    display.fillRect(x, y, cellWidth, cellHeight, WHITE);
  }

  // Draw rectangle border
  display.drawRect(x, y, cellWidth, cellHeight, isActiveStep ? BLACK : WHITE);

  // Draw cross
  uint8_t xEnd = x + cellWidth - 1; // Review what's up with pixel offset
  uint8_t yEnd = y + cellHeight - 1; // Review what's up with pixel offset
  display.drawLine(x, y, xEnd, yEnd, isActiveStep ? BLACK : WHITE);
  display.drawLine(x, yEnd, xEnd, y, isActiveStep ? BLACK : WHITE);
}

void displayBootScreen() {
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("PewBox");
  display.setTextSize(1);
  display.setTextColor(BLACK, WHITE); // 'inverted' text
  display.println("v0.6");
  display.display();
  delay(1000);
  display.clearDisplay();
}

void initAudioComponents() {
  AudioMemory(20); // Required - Reduce amount when needed quota is determined
  waveform1.frequency(440);
  waveform1.amplitude(1.0);
  waveform1.begin(WAVEFORM_SAWTOOTH);

  filter1.resonance(0.55);
  filter1.octaveControl(2.6); // up 2.6 octaves (4850 Hz) & down 2.6 octaves (132 Hz)

  filter_lfo.frequency(5);
  filter_lfo.amplitude(1.0);
  filter_lfo.begin(WAVEFORM_TRIANGLE);
}

void renderAudioComponentsFromMenu() {
  AudioNoInterrupts();
  waveform1.frequency(menuItems[0].value);
  waveform1.amplitude(menuItems[1].value);
  filter1.frequency(menuItems[2].value * 100); // Multiplying frequency for ease of use
  filter_lfo.frequency(menuItems[3].value);
  filter_lfo.amplitude(menuItems[4].value);
  AudioInterrupts();
}

// The operation to be performed on each clockwise encoder increment
// This allows the encoder action to be contextual,
// depending on which handler you pass
void menuControlClockwiseHandler() {
  // If we've stepped into the values of an item, increase the current value
  if (encoderToggled) {
    if (menuItems[activeMenuItemIndex].value < menuItems[activeMenuItemIndex].maxValue) {
      menuItems[activeMenuItemIndex].value++;
    }
  } else { // Otherwise keep scrolling through the menu list
   if (encoderValue < encoderValueMax) {
      encoderValue++; // Scroll to the next menu item
    }
  }
}

// The operation to be performed on each conterclockwise encoder increment
// This allows the encoder action to be contextual,
// depending on which handler you pass
void menuControlCounterclockwiseHandler() {
  // If we've stepped into the values of an item, decrease the current value
  if (encoderToggled) {
    if (menuItems[activeMenuItemIndex].value > menuItems[activeMenuItemIndex].minValue) {
      menuItems[activeMenuItemIndex].value--;
    }
  } else { // Otherwise keep scrolling through the menu list
    if (encoderValue > encoderValueMin) {
      encoderValue--; // Scroll to the previous menu item
    }
  }
}
