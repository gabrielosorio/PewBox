// Grid sequencer
// Managed by a bitmap describing the sequence length and active steps
//
// E.g:
//
// uint8_t gridRows = 2;
// uint8_t gridColumns = 8;
// unsigned char bitmap[gridRows] = {
//   B00100101,
//   B10001000
// };
//
// Rows are traversed top-down
// Bits are traversed back-to-front
// gridColumns is used by drawGridFromBitmap to establish boundaries

// Output indicator LED
#define LED 6

uint8_t stepTicker = 0;
uint8_t cursorIndex = 0;
const uint8_t gridRows = 2;
const uint8_t gridColumns = 8;

// TODO: Review passing by reference vs by pointer
unsigned char bitmap[gridRows] = {
  B00100101,
  B10001000
}; // Bits are traversed back-to-front

void initSequencer() {
  pinMode(LED, OUTPUT);
}

void sequencerControlClockwiseHandler() {
  if (cursorIndex < gridRows * gridColumns - 1) {
    cursorIndex++;
  }
}

void sequencerControlCounterclockwiseHandler() {
  if (cursorIndex > 0) {
    cursorIndex--;
  }
}

void flipBitmapBitAtCursor() {
  // TODO: Simplify check per row

  if (cursorIndex < 8) {
    // If on first row
    bitmap[0] ^= 1UL << cursorIndex; // Flip bit at cursorIndex
  } else {
    // If on second row
    bitmap[1] ^= 1UL << cursorIndex - 8; // Flip bit at cursorIndex
  }
}

void sequencerControlSwitchMomentaryHandler() {
  flipBitmapBitAtCursor();
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

void drawCursor(uint8_t x, uint8_t y, uint8_t cellWidth, uint8_t cellHeight, bool isActiveStep) {
  uint8_t cursorSize = 4;
  uint8_t cursorX = x + cellWidth / 2 - cursorSize / 2;
  uint8_t cursorY = y + cellHeight / 2 - cursorSize / 2;
  display.fillRect(cursorX, cursorY, cursorSize, cursorSize, isActiveStep ? BLACK : WHITE);
}

void drawUnmarkedCell(uint8_t x, uint8_t y, uint8_t cellWidth, uint8_t cellHeight, bool isActiveStep, bool hasCursorOnStep) {
  // Fill rectangle if the step is currently active
  if (isActiveStep) {
    // Draw filled rectangle
    display.fillRect(x, y, cellWidth, cellHeight, WHITE);
  }

  // Draw rectangle border
  display.drawRect(x, y, cellWidth, cellHeight, isActiveStep ? BLACK : WHITE);

  if (hasCursorOnStep) {
    drawCursor(x, y, cellWidth, cellHeight, isActiveStep);
  }
}

void drawMarkedCell(uint8_t x, uint8_t y, uint8_t cellWidth, uint8_t cellHeight, bool isActiveStep, bool hasCursorOnStep) {
  // Fill rectangle if the step is currently active
  if (isActiveStep) {
    // Draw filled rectangle
    display.fillRect(x, y, cellWidth, cellHeight, WHITE);
  }

  // Draw rectangle border
  display.drawRect(x, y, cellWidth, cellHeight, isActiveStep ? BLACK : WHITE);

  // Draw cross
  uint8_t xEnd = x + cellWidth - 1;   // Review what's up with pixel offset
  uint8_t yEnd = y + cellHeight - 1;  // Review what's up with pixel offset
  display.drawLine(x, y, xEnd, yEnd, isActiveStep ? BLACK : WHITE);
  display.drawLine(x, yEnd, xEnd, y, isActiveStep ? BLACK : WHITE);

  if (hasCursorOnStep) {
    drawCursor(x, y, cellWidth, cellHeight, isActiveStep);
  }
}

void drawGridFromBitmap(uint8_t activeStepIndex, uint8_t cursorIndex, uint8_t *bitmap, uint8_t gridRows, uint8_t gridColumns) {  // Technically bitmap or just byte array?
  uint8_t cellSize = 10;

  // TODO: Control layer (showing cursor position)
  // Only one active at a time, can infer from step number

  // TODO: Playback layer (showing active step position)
  // Only one active at a time, can infer from step number

  // Go through the rows
  for (uint8_t row = 0; row < gridRows; row++) {
    uint8_t cellOffsetY = row * cellSize;

    // Go through the columns in the row
    for (uint8_t column = 0; column < gridColumns; column++) {
      uint8_t cellOffsetX = column * cellSize;

      byte currentBit = bitRead(bitmap[row], column);  // needs to pass *bitmap if not already a pointer

      uint8_t currentStepIndex = gridColumns * row + column;
      bool isActiveStep = activeStepIndex == currentStepIndex;
      bool hasCursorOnStep = cursorIndex == currentStepIndex;

      if (currentBit == 1) {
        if (isActiveStep) {
          handleCurrentStepOn();
        }
        drawMarkedCell(cellOffsetX, cellOffsetY, cellSize, cellSize, isActiveStep, hasCursorOnStep);
      } else {
        if (isActiveStep) {
          handleCurrentStepOff();
        }
        drawUnmarkedCell(cellOffsetX, cellOffsetY, cellSize, cellSize, isActiveStep, hasCursorOnStep);
      }
    }
  }
}

uint8_t sequencerTickPeriod = 1000;
unsigned long currentTime = 0;

// Main sequencer entry point
// Put this in the run loop
void renderSequencer() {
  display.clearDisplay();
  drawGridFromBitmap(stepTicker, cursorIndex, bitmap, gridRows, gridColumns);
  display.display();

  // Delay using millis to avoid blocking UI between ticks
  if (millis() >= currentTime + sequencerTickPeriod) {
    currentTime += sequencerTickPeriod;

    // Dummy Step Cycle
    if (stepTicker == 15) {
      stepTicker = 0;
    } else {
      stepTicker++;
    }
  }
}