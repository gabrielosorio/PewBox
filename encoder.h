// Rotary Encoder

// * Pins:
// Defined in PewBox.ino
// #define ENCODER_INPUT_CLK 2
// #define ENCODER_INPUT_DT 3
// #define ENCODER_INPUT_SW 4

// * Setup functions:
// initRotaryEncoder();

// * Handlers:
//
// Pass handler function to be invoked every time
// the encoder switch is pressed or released
// readEncoderSwitch(*foobarControlSwitchMomentaryHandler);
//
// Pass handler functions to be invoked every time
// the encoder latches in each respective direction
// readEncoderRotation(
//   *foobarControlClockwiseHandler,
//   *foobarControlCounterclockwiseHandler
// );

#ifndef ENCODER_H
#define ENCODER_H

#include <Arduino.h>

class Encoder {
public:
  Encoder(int clkPin, int dtPin, int swPin);
  ~Encoder();

  int initRotaryEncoder();

  bool toggled();
  bool momentaryOn();

  void readEncoderRotation(
    void (*clockwiseHandler)(),
    void (*counterclockwiseHandler)()
  );
  void readEncoderSwitch(void (*encoderSwitchMomentaryHandler)());
  void readEncoderSwitchMomentary(void (*encoderSwitchMomentaryHandler)());

private:
  unsigned int clk_pin;
  unsigned int dt_pin;
  unsigned int sw_pin;
  int encoderCurrentCLK;
  int encoderPreviousCLK;
  int encoderCurrentSW;
  int encoderPreviousSW;

  bool currentlyToggled = false;
  bool currentlyMomentaryOn = false;
};

#endif // ENCODER_H
