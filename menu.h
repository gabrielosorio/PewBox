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

uint8_t activeMenuItemIndex = 0;

struct valueMenuItem {
  char *label;
  uint8_t value;
  uint8_t minValue;
  uint8_t maxValue;
};

valueMenuItem menuItems[MENU_SIZE];

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

  menuItems[3].label = "Filter LFO Freq";
  menuItems[3].value = 5;
  menuItems[3].minValue = 0;
  menuItems[3].maxValue = 255;

  menuItems[4].label = "Filter LFO On/Off";
  menuItems[4].value = 0;
  menuItems[4].minValue = 0;
  menuItems[4].maxValue = 1;
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

void renderMenu(uint8_t nextMenuItemIndex) {
  activeMenuItemIndex = nextMenuItemIndex;
  
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
