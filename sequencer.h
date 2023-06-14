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
#define TRACK_0_LED 35
#define TRACK_1_LED 36
#define TRACK_2_LED 37
#define TRACK_3_LED 38

uint8_t stepTicker = 0;
uint8_t cursorIndex = 0;
const uint8_t gridRows = 8;     // TODO: Rename to describe bitmap and abstract
const uint8_t gridColumns = 8;  //       from how they're displayed
const uint8_t displayStepsPerRow = 16;
uint8_t cellSize = 7;
uint8_t borderWidth = 1;

uint8_t internalTempo = 120; // bpm
uint8_t sequencerTickPeriod = 60000 / internalTempo; // bpm to milliseconds
unsigned long currentTime = 0;
uint8_t trigLength = 20;

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
  pinMode(TRACK_0_LED, OUTPUT);
  pinMode(TRACK_1_LED, OUTPUT);
  pinMode(TRACK_2_LED, OUTPUT);
  pinMode(TRACK_3_LED, OUTPUT);
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
  uint8_t bitmapRow = cursorIndex / gridColumns;
  // Flip bit at relative cursor index for current bitmap row
  bitmap[bitmapRow] ^= 1UL << (cursorIndex % gridColumns);
}

void sequencerControlSwitchMomentaryHandler() {
  flipBitmapBitAtCursor();
}

// Sequencer event stepping on an enabled trigger
void handleStepOnForTrack(uint8_t track) {
  switch(track) {
    case 0:
      digitalWrite(TRACK_0_LED, HIGH);
      break;
    case 1:
      digitalWrite(TRACK_1_LED, HIGH);
      break;
    case 2:
      digitalWrite(TRACK_2_LED, HIGH);
      break;
    case 3:
      digitalWrite(TRACK_3_LED, HIGH);
      break;
  }
}

// Sequencer event stepping on a disabled trigger
void handleStepOffForTrack(uint8_t track) {
  switch(track) {
    case 0:
      digitalWrite(TRACK_0_LED, LOW);
      break;
    case 1:
      digitalWrite(TRACK_1_LED, LOW);
      break;
    case 2:
      digitalWrite(TRACK_2_LED, LOW);
      break;
    case 3:
      digitalWrite(TRACK_3_LED, LOW);
      break;
  }
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

void drawGridFromBitmap(uint8_t xPos, uint8_t yPos, uint8_t activeStepIndex, uint8_t cursorIndex, uint8_t *bitmap, uint8_t gridRows, uint8_t gridColumns) {  // Technically bitmap or just byte array?
  // TODO: Control layer (showing cursor position)
  // Only one active at a time, can infer from step number

  // TODO: Playback layer (showing active step position)
  // Only one active at a time, can infer from step number

  // Go through the rows
  for (uint8_t row = 0; row < gridRows; row++) {
    // Go through the columns in the row
    for (uint8_t column = 0; column < gridColumns; column++) {
      uint8_t cellOffsetY = yPos + (gridColumns * row + column) / displayStepsPerRow * cellSize; // Offset 1 Y after 16 steps
      uint8_t cellOffsetX = xPos + ((gridColumns * row + column) % displayStepsPerRow) * cellSize; // Offset 1 X up until 16 steps

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
          handleStepOnForTrack(currentTrack);

          // Shorten the gate into a trig
          // Also avoids it staying open when a following adjacent trig is fired
          if (millis() >= currentTime + trigLength) {
            handleStepOffForTrack(currentTrack);
          }
        }
        drawMarkedCell(cellOffsetX, cellOffsetY, cellSize, cellSize, isActiveStep, hasCursorOnStep);
      } else {
        if (isActiveStep) {
          handleStepOffForTrack(currentTrack);
        }
        drawUnmarkedCell(cellOffsetX, cellOffsetY, cellSize, cellSize, isActiveStep, hasCursorOnStep);
      }
    }
  }
}

void displayTempo(uint8_t x, uint8_t y) {
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(x, y);
  display.print(internalTempo);
  display.println(" BPM");
}

// Main sequencer entry point
// Put this in the run loop
void renderSequencer() {
  display.clearDisplay();
  displayTempo(80, 4);
  drawGridFromBitmap(8, 16, stepTicker, cursorIndex, bitmap, gridRows, gridColumns);
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