// Menu
// Implements interface functions

#include "menu.h"

Menu::Menu(Adafruit_SSD1305 display) {
  this->display = display;
};

Menu::~Menu() {
};

void Menu::initMenu() {
  this->menuItems[0].label = "Oscillator Freq";
  this->menuItems[0].value = 195;
  this->menuItems[0].minValue = 0;
  this->menuItems[0].maxValue = 255;

  this->menuItems[1].label = "Oscillator On/Off";
  this->menuItems[1].value = 0;
  this->menuItems[1].minValue = 0;
  this->menuItems[1].maxValue = 1;

  this->menuItems[2].label = "Filter Freq (MHz)";
  this->menuItems[2].value = 45;
  this->menuItems[2].minValue = 0;
  this->menuItems[2].maxValue = 255;

  this->menuItems[3].label = "Filter LFO Freq";
  this->menuItems[3].value = 5;
  this->menuItems[3].minValue = 0;
  this->menuItems[3].maxValue = 255;

  this->menuItems[4].label = "Filter LFO On/Off";
  this->menuItems[4].value = 0;
  this->menuItems[4].minValue = 0;
  this->menuItems[4].maxValue = 1;
}

void Menu::highlightSelectedMenuItemLabel(uint8_t renderingMenuItemIndex) {
  // If we've stepped into the values of an item, skip
  if (valueLevelSelected) {
    return;
  }

  // If we're rendering the currently selected menu item, highlight it
  if (renderingMenuItemIndex == activeMenuItemIndex) {
    display.setTextColor(BLACK, WHITE); // 'inverted' text
  } else {
    display.setTextColor(WHITE);
  }
}

void Menu::highlightSelectedMenuItemValue(uint8_t renderingMenuItemIndex) {
  // If we have not stepped into the values of an item, skip
  if (!valueLevelSelected) {
    return;
  }

  // If we're rendering the currently selected menu item, highlight it
  if (renderingMenuItemIndex == activeMenuItemIndex) {
    display.setTextColor(BLACK, WHITE); // 'inverted' text
  } else {
    display.setTextColor(WHITE);
  }
}

void Menu::renderMenu() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);

  for (uint8_t i = 0; i < this->menuSize; i++) {
    display.setTextColor(WHITE);

    // Highlight item if selected
    highlightSelectedMenuItemLabel(i);

    display.print(menuItems[i].label);
    display.print(": "); // Spacer

    highlightSelectedMenuItemValue(i);
    display.println(menuItems[i].value);
  }

  display.display();
}

void Menu::toggleSelectedLevel() {
  valueLevelSelected = !valueLevelSelected;
}
