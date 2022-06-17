// Rotary Encoder
// Still tightly coupled to the menu and with
// very little encapsulation

// # Pins:
// Defined in PewBox.ino
// #define ENCODER_INPUT_CLK 2
// #define ENCODER_INPUT_DT 3
// #define ENCODER_INPUT_SW 4

// # Control variables:
// encoderToggled

// # Setup functions:
// initRotaryEncoder();

// # Loop functions:
//
// Keep encoder button in sync
// readEncoderSwitch();
//
// Pass handler functions to be invoked every time
// the encoder latches in each respective direction
// readEncoderRotation(
//   *menuControlClockwiseHandler,
//   *menuControlCounterclockwiseHandler
// );

// Handlers:
// Invoked every time the encoder latches in each
// respective direction
//
// void menuControlCounterclockwiseHandler()
// void menuControlClockwiseHandler()

int encoderCurrentCLK;
int encoderPreviousCLK;
int encoderCurrentSW;
int encoderPreviousSW;
bool encoderToggled = false;

void initRotaryEncoder() {
  // Init Rotary Encoder
  pinMode(ENCODER_INPUT_CLK, INPUT);
  pinMode(ENCODER_INPUT_DT, INPUT);
  pinMode(ENCODER_INPUT_SW, INPUT);
  encoderPreviousCLK = digitalRead(ENCODER_INPUT_CLK); // Initial CLK state
  encoderPreviousSW = digitalRead(ENCODER_INPUT_SW); // Initial SW state
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
