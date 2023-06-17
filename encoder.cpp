// Rotary Encoder
// Implements interface functions

#include "encoder.h"

Encoder::~Encoder() {
};

Encoder::Encoder(int clkPin, int dtPin, int swPin) {
  this->clk_pin = clkPin;
  this->dt_pin = dtPin;
  this->sw_pin = swPin;
};

int Encoder::initRotaryEncoder() {
  pinMode(this->clk_pin, INPUT);
  pinMode(this->dt_pin, INPUT);
  pinMode(this->sw_pin, INPUT);

  encoderPreviousCLK = digitalRead(this->clk_pin); // Initial CLK state
  encoderPreviousSW = digitalRead(this->sw_pin); // Initial SW state

  return 0;
}

// Getters & Setters

bool Encoder::toggled() {
  return currentlyToggled;
}

bool Encoder::momentaryOn() {
  return currentlyMomentaryOn;
}

// Public Methods

void Encoder::readEncoderSwitch(void (*encoderSwitchMomentaryHandler)()) {
  encoderCurrentSW = digitalRead(this->sw_pin);

  // If the SW didn't change, then the encoder wasn't pressed or released. Do nothing.
  if (encoderCurrentSW == encoderPreviousSW) {
    return;
  }

  if (encoderCurrentSW == LOW) {
    Serial.println("Encoder Pressed");
    currentlyToggled = !currentlyToggled;
    // Trigger a momentary handler if one is supplied
    encoderSwitchMomentaryHandler();
  }

  if (encoderCurrentSW == HIGH) {
    Serial.println("Encoder Released");
  }

  encoderPreviousSW = encoderCurrentSW;
}

// TODO: Make function argument optional
void Encoder::readEncoderSwitchMomentary(void (*encoderSwitchMomentaryHandler)()) {
  // If the SW didn't change, then the encoder wasn't pressed or released. Do nothing.
  if (encoderCurrentSW == encoderPreviousSW) {
    return;
  }

  if (encoderCurrentSW == LOW) {
    Serial.println("Encoder Pressed");
    encoderSwitchMomentaryHandler();
  }
}

void Encoder::readEncoderRotation(void (*clockwiseHandler)(), void (*counterclockwiseHandler)()) {
  encoderCurrentCLK = digitalRead(this->clk_pin);

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
  if (encoderCurrentCLK == digitalRead(this->dt_pin)) { // Counterclockwise
    counterclockwiseHandler();
  } else { // Clockwise
    clockwiseHandler();
  }
}
