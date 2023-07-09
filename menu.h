// Main Menu
// Still tightly coupled to the encoder controls
// very little encapsulation and very order dependent

// # Constants:
// Defined in PewBox.ino
// #define MENU_SIZE 5

// # Control variables:
// activeMenuItemIndex
// menuItems

// # Setup functions:
// initMenu();

// # Loop functions:
// renderMenu(index); // index to be set as active menu item

#ifndef MENU_H
#define MENU_H

#include <Arduino.h>

// TODO: Tidier way to avoid passing large instances?
// Used to receive the display instance to draw the menu
#include <Adafruit_SSD1305.h>

// TODO: Pass as init argument instead
#define MENU_SIZE 5  // 1-based count (one blank) - must be a constant/macro to be used to define an array

uint8_t activeMenuItemIndex = 0;

struct valueMenuItem {
  char *label;
  uint8_t value;
  uint8_t minValue;
  uint8_t maxValue;
};

class Menu {
public:
  // TODO: Make sure to receive by reference
  Menu(Adafruit_SSD1305 display);
  ~Menu();

  void initMenu();
  void renderMenu();
  void toggleSelectedLevel();

  int menuSize;
  valueMenuItem menuItems[MENU_SIZE];

private:
  Adafruit_SSD1305 display;
  bool valueLevelSelected = false;
  void highlightSelectedMenuItemLabel(uint8_t renderingMenuItemIndex);
  void highlightSelectedMenuItemValue(uint8_t renderingMenuItemIndex);
};

#endif // MENU_H
