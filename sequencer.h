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
const uint8_t gridRows = 8;     // TODO: Rename to describe bitmap and abstract
const uint8_t gridColumns = 8;  //       from how they're displayed
const uint8_t displayStepsPerRow = 16;
uint8_t cellSize = 7;
uint8_t borderWidth = 1;

// TODO: Review passing by reference vs by pointer
unsigned char bitmap[gridRows] = {
  B00100101,
  B10001000,
  B10011000,
  B00100001,
  B00100101,
  B10001000,
  B1000010,
  B01000110
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
  uint8_t innerPadding = 0;
  if (cellWidth >= 10) {
    // Use double pixel inner padding for larger sizes
    innerPadding = 2;
  } else {
    // Use single pixel inner padding for smaller sizes
    innerPadding = 1;
  }

  // Cursor size is
  // cell size - borders - paddings
  uint8_t cursorSize = cellWidth - (borderWidth + innerPadding) * 2;
  uint8_t cursorX = x + borderWidth + innerPadding;
  uint8_t cursorY = y + borderWidth + innerPadding;

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
  // TODO: Control layer (showing cursor position)
  // Only one active at a time, can infer from step number

  // TODO: Playback layer (showing active step position)
  // Only one active at a time, can infer from step number

  // Go through the rows
  for (uint8_t row = 0; row < gridRows; row++) {
    // Go through the columns in the row
    for (uint8_t column = 0; column < gridColumns; column++) {
      uint8_t cellOffsetY = (gridColumns * row + column) / displayStepsPerRow * cellSize; // Offset 1 Y after 16 steps
      uint8_t cellOffsetX = ((gridColumns * row + column) % displayStepsPerRow) * cellSize; // Offset 1 X up until 16 steps

      byte currentlyRenderingBit = bitRead(bitmap[row], column);  // needs to pass *bitmap if not already a pointer

      uint8_t currentlyRenderingStepIndex = gridColumns * row + column;

      // activeStepIndex is the step the sequencer is on, goes from 0 to 15
      // currentlyRenderingStepIndex is the step that is being drawn

      // One track per visual row (determined by displayStepsPerRow), goes from 0 to 3
      uint8_t currentTrack = (gridColumns * row + column) / displayStepsPerRow;

      // Check if is active for every track in the column
      bool isActiveStep = (currentlyRenderingStepIndex % displayStepsPerRow) == activeStepIndex;
      bool hasCursorOnStep = cursorIndex == currentlyRenderingStepIndex;

      if (currentlyRenderingBit == 1) {
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
    if (stepTicker == displayStepsPerRow - 1) {
      stepTicker = 0;
    } else {
      stepTicker++;
    }
  }
}