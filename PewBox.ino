#include <SPI.h>
#include <Adafruit_SSD1305.h>

// OLED SPI
#define OLED_CS 10
#define OLED_DC 9
#define OLED_RESET 8

// hardware SPI - use 7Mhz (7000000UL) or lower because the screen is rated for 4MHz, or it will remain blank!
Adafruit_SSD1305 display(128, 64, &SPI, OLED_DC, OLED_RESET, OLED_CS, 1000000UL);

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

// Main Menu
uint8_t activeMenuItemIndex = 0;

struct valueMenuItem {
  char *label;
  uint8_t value;
  uint8_t minValue;
  uint8_t maxValue;
};

#define MENU_SIZE 3 // 3 items (one blank) - must be a constant/macro to be used to define an array
valueMenuItem menuItems[MENU_SIZE];

// Teensy Audio Library
#include <Audio.h>
#include <Wire.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform1;      //xy=396,277
AudioFilterLadder        filter1;        //xy=574,315
AudioOutputAnalog        dac1;           //xy=752,315
AudioConnection          patchCord1(waveform1, 0, filter1, 0);
AudioConnection          patchCord2(filter1, dac1);
// GUItool: end automatically generated code


void setup() {
  Serial.begin(9600);
  // Make sure serial is online before proceeding
  while (! Serial) delay(100);

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
  display.println("v0.5");
  display.display();
  delay(1000);
  display.clearDisplay();

  // Menu Setup
  initMenu();

  // Audio
  AudioMemory(20); // Required - Reduce amount when needed quota is determined
  waveform1.frequency(440);
  waveform1.amplitude(1.0);
  waveform1.begin(WAVEFORM_SAWTOOTH);
}

void loop() {
  readEncoderSwitch();
  readEncoderRotation(
    *menuControlClockwiseHandler,
    *menuControlCounterclockwiseHandler
  );

  // Read selected meenu item from encoder
  activeMenuItemIndex = encoderValue;
  renderMenu();

  display.display();
  display.clearDisplay();

  // Audio
  AudioNoInterrupts();
  waveform1.frequency(menuItems[0].value);
  waveform1.amplitude(menuItems[1].value);
  filter1.resonance(0.55);
  filter1.frequency(menuItems[2].value * 100); // Multiplying frequency for ease of use
  filter1.octaveControl(2.6); // up 2.6 octaves (4850 Hz) & down 2.6 octaves (132 Hz)
  AudioInterrupts();
}

void initMenu() {
  menuItems[0].label = "Oscillator Freq";
  menuItems[0].value = 195;
  menuItems[0].minValue = 0;
  menuItems[0].maxValue = 255;

  menuItems[1].label = "Oscillator On/Off";
  menuItems[1].value = 0;
  menuItems[1].minValue = 0;
  menuItems[1].maxValue = 1;

  menuItems[2].label = "Filter Freq (MHz)";
  menuItems[2].value = 45;
  menuItems[2].minValue = 0;
  menuItems[2].maxValue = 255;
}

void renderMenu() {
  display.setCursor(0, 0);
  display.setTextSize(1);

  for (uint8_t i = 0; i < MENU_SIZE; i++) {
    display.setTextColor(WHITE);

    // Highlight item if selected
    highlightSelectedMenuItemLabel(i);

    display.print(menuItems[i].label);
    display.print(": "); // Spacer

    highlightSelectedMenuItemValue(i);
    display.println(menuItems[i].value);
  }
}

void highlightSelectedMenuItemLabel(uint8_t renderingMenuItemIndex) {
  // If we've stepped into the values of an item, skip
  if (encoderToggled) {
    return;
  }

  // If we're rendering the currently selected menu item, highlight it
  if (renderingMenuItemIndex == activeMenuItemIndex) {
    display.setTextColor(BLACK, WHITE); // 'inverted' text
  } else {
    display.setTextColor(WHITE);
  }
}

void highlightSelectedMenuItemValue(uint8_t renderingMenuItemIndex) {
  // If we have not stepped into the values of an item, skip
  if (!encoderToggled) {
    return;
  }

  // If we're rendering the currently selected menu item, highlight it
  if (renderingMenuItemIndex == activeMenuItemIndex) {
    display.setTextColor(BLACK, WHITE); // 'inverted' text
  } else {
    display.setTextColor(WHITE);
  }
}

void readEncoderSwitch() {
  encoderCurrentSW = digitalRead(ENCODER_INPUT_SW);

  // If the SW didn't change, then the encoder wasn't pressed or released. Do nothing.
  if (encoderCurrentSW == encoderPreviousSW) {
    return;
  }

  if (encoderCurrentSW == LOW) {
    Serial.println("Encoder Pressed");
    encoderToggled = !encoderToggled;
  }

  if (encoderCurrentSW == HIGH) {
    Serial.println("Encoder Released");
  }

  encoderPreviousSW = encoderCurrentSW;
}

void readEncoderRotation(void (*clockwiseHandler)(), void (*counterclockwiseHandler)()) {
  encoderCurrentCLK = digitalRead(ENCODER_INPUT_CLK);

  // If the CLK didn't change, then the encoder didn't move. Do nothing.
  if (encoderCurrentCLK == encoderPreviousCLK) {
    return;
  }

  encoderPreviousCLK = encoderCurrentCLK;

  // Extremely basic debounce.
  // Current encoder only latches on 1 and reads 0 when not latched.
  // Only compute rotation when the encoder latches.
  if (encoderCurrentCLK == 0) {
    return;
  }

  // Both CLK and DT are HIGH when rotating counterclockwise
  if (encoderCurrentCLK == digitalRead(ENCODER_INPUT_DT)) { // Counterclockwise
    counterclockwiseHandler();
  } else { // Clockwise
    clockwiseHandler();
  }
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
