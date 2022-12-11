/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Handler of the Config page                                      *
 * Author:   Ronan Demoment                                                *
 *                                                                         *
 ***************************************************************************
 *   Copyright (C) 2022 by Ronan Demoment                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */

/***************************************************************************/
/*                              Includes                                   */
/***************************************************************************/

#include "ConfigPage1.h"
#include "Globals.h"
#include "MicronetDevice.h"
#include "PanelResources.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

#define NUMBER_OF_CONFIG_ITEMS 4
#define SELECTION_X_POSITION 72

/***************************************************************************/
/*                             Local types                                 */
/***************************************************************************/

/***************************************************************************/
/*                           Local prototypes                              */
/***************************************************************************/

/***************************************************************************/
/*                           Static & Globals                              */
/***************************************************************************/

/***************************************************************************/
/*                              Functions                                  */
/***************************************************************************/

ConfigPage1::ConfigPage1()
    : editMode(false), editPosition(0), configFreqSel(0), configNmeaSel(0),
      configRmbWorkaround(false), configWindRepeater(false) {}

ConfigPage1::~ConfigPage1() {}

void ConfigPage1::Draw(bool force) {
  char versionStr[10];
  char networkIdStr[9];
  int16_t xVersion, yVersion;
  uint16_t wVersion, hVersion;

  if (force) {
    configFreqSel = (uint32_t)gConfiguration.eeprom.freqSystem;
    configNmeaSel = (uint32_t)gConfiguration.eeprom.nmeaLink;
    configRmbWorkaround = gConfiguration.eeprom.rmbWorkaround;
    configWindRepeater = gConfiguration.eeprom.windRepeater;
  }

  if (display != nullptr) {
    display->clearDisplay();

    display->setTextColor(SSD1306_WHITE);
    display->setTextSize(1);
    display->setFont(nullptr);
    display->setCursor(0, 0);
    display->println("Frequency");
    display->println("NMEA");
    display->println("RMB bugfix");
    display->println("Wind Repeat");

    for (int i = 0; i < NUMBER_OF_CONFIG_ITEMS; i++) {
      display->setCursor(SELECTION_X_POSITION, i * 8);
      if (editMode && (editPosition == i)) {
        display->fillRect(SELECTION_X_POSITION, i * 8,
                          SCREEN_WIDTH - SELECTION_X_POSITION, 8,
                          SSD1306_WHITE);
        display->setTextColor(SSD1306_BLACK);
      } else {
        display->setTextColor(SSD1306_WHITE);
      }
      display->print(ConfigString(i));
    }

    if (editMode && (editPosition == NUMBER_OF_CONFIG_ITEMS)) {
      display->fillRect(31, 64 - 8, 11 * 6, 8, SSD1306_WHITE);
      display->setTextColor(SSD1306_BLACK);
    } else {
      display->setTextColor(SSD1306_WHITE);
    }
    if (editMode) {
      display->setCursor(31, 64 - 8);
      display->print("Save & Exit");
    }
    display->display();
  }
}

PageAction_t ConfigPage1::OnButtonPressed(bool longPress) {
  PageAction_t action = PAGE_ACTION_NEXT_PAGE;

  if (editMode) {
    if (longPress) {
      if (editPosition == NUMBER_OF_CONFIG_ITEMS) {
        editMode = false;
        DeployConfiguration();
      } else {
        ConfigCycle(editPosition);
      }
      action = PAGE_ACTION_REFRESH;
    } else {
      editPosition = (editPosition + 1) % (NUMBER_OF_CONFIG_ITEMS + 1);
      action = PAGE_ACTION_REFRESH;
    }
  } else {
    if (longPress) {
      editMode = true;
      editPosition = 0;
      action = PAGE_ACTION_REFRESH;
    } else {
      action = PAGE_ACTION_NEXT_PAGE;
    }
  }

  return action;
}

char const *ConfigPage1::ConfigString(uint32_t index) {
  switch (index) {
  case 0:
    return ConfigFreqString();
  case 1:
    return ConfigNmeaString();
  case 2:
    return ConfigRmbWorkaroundString();
  case 3:
    return ConfigWindRepeaterString();
  }

  return "---";
}

char const *ConfigPage1::ConfigFreqString() {
  switch (configFreqSel) {
  case 0:
    return "868Mhz";
  case 1:
    return "915Mhz";
  }

  return "---";
}

char const *ConfigPage1::ConfigNmeaString() {
  switch (configNmeaSel) {
  case 0:
    return "USB";
  case 1:
    return "Bluetooth";
  case 3:
    return "WiFi";
  }

  return "---";
}

char const *ConfigPage1::ConfigRmbWorkaroundString() {
  if (configRmbWorkaround) {
    return "Yes";
  }
  return "No";
}

char const *ConfigPage1::ConfigWindRepeaterString() {
  if (configWindRepeater) {
    return "Yes";
  }
  return "No";
}

void ConfigPage1::ConfigCycle(uint32_t index) {
  switch (index) {
  case 0:
    ConfigFreqCycle();
    break;
  case 1:
    ConfigNmeaCycle();
    break;
  case 2:
    ConfigRmbWorkaroundCycle();
    break;
  case 3:
    ConfigWindRepeaterCycle();
    break;
  }
}

void ConfigPage1::ConfigFreqCycle() { configFreqSel = (configFreqSel + 1) % 2; }

void ConfigPage1::ConfigNmeaCycle() { configNmeaSel = (configNmeaSel + 1) % 2; }

void ConfigPage1::ConfigRmbWorkaroundCycle() {
  configRmbWorkaround = !configRmbWorkaround;
}

void ConfigPage1::ConfigWindRepeaterCycle() {
  configWindRepeater = !configWindRepeater;
}

void ConfigPage1::DeployConfiguration() {
  gConfiguration.eeprom.freqSystem = (FreqSystem_t)configFreqSel;
  gConfiguration.eeprom.nmeaLink = (SerialType_t)configNmeaSel;
  gConfiguration.eeprom.rmbWorkaround = configRmbWorkaround;
  gConfiguration.eeprom.windRepeater = configWindRepeater;

  gConfiguration.DeployConfiguration(&gMicronetDevice);
  gConfiguration.SaveToEeprom();
}
