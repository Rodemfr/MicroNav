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

#include "ConfigPage2.h"
#include "Globals.h"
#include "MicronetDevice.h"
#include "PanelResources.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

// @brief Number of configuration items on this page
#define NUMBER_OF_CONFIG_ITEMS 5
// @brief Horizontal position of configuration values on display
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

ConfigPage2::ConfigPage2()
    : editMode(false), editPosition(0), configCompassSel(0), configGnssSel(0), configWindSel(0), configDepthSel(0), configSpeedSel(0)
{
}

ConfigPage2::~ConfigPage2()
{
}

// @brief Draw the page on display
// @param force Force redraw, even if the content did not change
void ConfigPage2::Draw(bool force, bool flushDisplay)
{
    int16_t  xStr, yStr;
    uint16_t wStr, hStr;

    // Forced draw occurs when user enters the page : load configuration locally
    if (force && !editMode)
    {
        // Convert compass enum
        switch (gConfiguration.eeprom.compassSource)
        {
        case LINK_MICRONET:
            configCompassSel = 0;
            break;
        case LINK_COMPASS:
            configCompassSel = 1;
            break;
        case LINK_NMEA_EXT:
            configCompassSel = 2;
            break;
        }

        // Convert GNSS enum
        switch (gConfiguration.eeprom.gnssSource)
        {
        case LINK_NMEA_GNSS:
            configGnssSel = 0;
            break;
        case LINK_NMEA_EXT:
            configGnssSel = 1;
            break;
        }

        // Convert wind enum
        switch (gConfiguration.eeprom.windSource)
        {
        case LINK_MICRONET:
            configWindSel = 0;
            break;
        case LINK_NMEA_EXT:
            configWindSel = 1;
            break;
        }

        // Convert depth enum
        switch (gConfiguration.eeprom.depthSource)
        {
        case LINK_MICRONET:
            configDepthSel = 0;
            break;
        case LINK_NMEA_EXT:
            configDepthSel = 1;
            break;
        }

        // Convert speed enum
        switch (gConfiguration.eeprom.speedSource)
        {
        case LINK_MICRONET:
            configSpeedSel = 0;
            break;
        case LINK_NMEA_EXT:
            configSpeedSel = 1;
            break;
        }
    }

    if (display != nullptr)
    {
        display->clearDisplay();

        // Config items
        display->setTextColor(SSD1306_WHITE);
        display->setTextSize(1);
        display->setFont(nullptr);
        display->setCursor(0, 0);
        display->println("Compass");
        display->println("GNSS");
        display->println("Wind");
        display->println("Depth");
        display->println("Speed");

        // Config values
        for (int i = 0; i < NUMBER_OF_CONFIG_ITEMS; i++)
        {
            display->setCursor(SELECTION_X_POSITION, i * 8);
            if (editMode && (editPosition == i))
            {
                display->fillRect(SELECTION_X_POSITION, i * 8, SCREEN_WIDTH - SELECTION_X_POSITION, 8, SSD1306_WHITE);
                display->setTextColor(SSD1306_BLACK);
            }
            else
            {
                display->setTextColor(SSD1306_WHITE);
            }
            display->getTextBounds(String(ConfigString(i)), 0, 0, &xStr, &yStr, &wStr, &hStr);
            display->setCursor(SCREEN_WIDTH - wStr, i * 8);
            display->print(ConfigString(i));
        }

        if (editMode)
        {
            if (editPosition == NUMBER_OF_CONFIG_ITEMS)
            {
                display->fillRect(104, 64 - 16, 4 * 6, 8, SSD1306_WHITE);
                display->setTextColor(SSD1306_BLACK);
            }
            else
            {
                display->setTextColor(SSD1306_WHITE);
            }
            display->setCursor(104, 64 - 16);
            display->print("Save");
        }

        if (flushDisplay)
        {
            display->display();
        }
    }
}

// @brief Function called by PanelManager when the button is pressed
// @param longPress true if a long press was detected
// @return Action to be executed by PanelManager
PageAction_t ConfigPage2::OnButtonPressed(ButtonId_t buttonId, bool longPress)
{
    PageAction_t action = PAGE_ACTION_NONE;

    if (editMode)
    {
        // In edit mode, the power button is used to cycle through the configuration items
        if ((buttonId == BUTTON_ID_0) && !longPress)
        {
            if (editPosition == NUMBER_OF_CONFIG_ITEMS)
            {
                // Long press on "Save"
                editMode = false;
                // Apply configuration
                DeployConfiguration();
            }
            else
            {
                // Press on a configuration item : cycle its value
                ConfigCycle(editPosition);
            }
            action = PAGE_ACTION_REFRESH;
        }
        else if ((buttonId == BUTTON_ID_1) && !longPress)
        {
            // Short press : cycle through configuration items
            editPosition = (editPosition + 1) % (NUMBER_OF_CONFIG_ITEMS + 1);
            action       = PAGE_ACTION_REFRESH;
        }
    }
    else
    {
        if ((buttonId == BUTTON_ID_1) && longPress)
        {
            editMode     = true;
            editPosition = 0;
            action       = PAGE_ACTION_REFRESH;
        }
        else if ((buttonId == BUTTON_ID_0) && !longPress)
        {
            action = PAGE_ACTION_EXIT_TOPIC;
        }
        else if ((buttonId == BUTTON_ID_1) && !longPress)
        {
            action = PAGE_ACTION_EXIT_PAGE;
        }
    }

    return action;
}

// @brief Return the string of a given configuration item
// @param index Configuration item
// @return String naming the value of the configuration item
char const *ConfigPage2::ConfigString(uint32_t index)
{
    switch (index)
    {
    case 0:
        return ConfigCompassString();
    case 1:
        return ConfigGnssString();
    case 2:
        return ConfigWindString();
    case 3:
        return ConfigDepthString();
    case 4:
        return ConfigSpeedString();
    }

    return "---";
}

// @brief Return the string of a the compass configuration item
// @return String naming the value of the compass source
char const *ConfigPage2::ConfigCompassString()
{
    switch (configCompassSel)
    {
    case 0:
        return "Micronet";
    case 1:
        return "LSM303";
    case 2:
        return "NMEA";
    }

    return "---";
}

// Return the string of a the GNSS configuration item
// @return String naming the value of the GNSS source
char const *ConfigPage2::ConfigGnssString()
{
    switch (configGnssSel)
    {
    case 0:
        return "NEO-6M";
    case 1:
        return "NMEA";
    }

    return "---";
}

// Return the string of a the Wind configuration item
// @return String naming the value of the Wind source
char const *ConfigPage2::ConfigWindString()
{
    switch (configWindSel)
    {
    case 0:
        return "Micronet";
    case 1:
        return "NMEA";
    }

    return "---";
}

// Return the string of a the depth configuration item
// @return String naming the value of the depth source
char const *ConfigPage2::ConfigDepthString()
{
    switch (configDepthSel)
    {
    case 0:
        return "Micronet";
    case 1:
        return "NMEA";
    }

    return "---";
}

// Return the string of a the speed configuration item
// @return String naming the value of the speed source
char const *ConfigPage2::ConfigSpeedString()
{
    switch (configSpeedSel)
    {
    case 0:
        return "Micronet";
    case 1:
        return "NMEA";
    }

    return "---";
}

// @brief Cycle the value of a given configuration item
// @param index Configuration item
void ConfigPage2::ConfigCycle(uint32_t index)
{
    switch (index)
    {
    case 0:
        ConfigCompassCycle();
        break;
    case 1:
        ConfigGnssCycle();
        break;
    case 2:
        ConfigWindCycle();
        break;
    case 3:
        ConfigDepthCycle();
        break;
    case 4:
        ConfigSpeedCycle();
        break;
    }
}

// @brief Cycle the value of the compass configuration item
void ConfigPage2::ConfigCompassCycle()
{
    configCompassSel = (configCompassSel + 1) % 3;
}

// @brief Cycle the value of the GNSS configuration item
void ConfigPage2::ConfigGnssCycle()
{
    configGnssSel = (configGnssSel + 1) % 2;
}

// @brief Cycle the value of the wind configuration item
void ConfigPage2::ConfigWindCycle()
{
    configWindSel = (configWindSel + 1) % 2;
}

// @brief Cycle the value of the depth configuration item
void ConfigPage2::ConfigDepthCycle()
{
    configDepthSel = (configDepthSel + 1) % 2;
}

// @brief Cycle the value of the speed configuration item
void ConfigPage2::ConfigSpeedCycle()
{
    configSpeedSel = (configSpeedSel + 1) % 2;
}

// @brief Deploy the local configuration to the overall system and save it to
// EEPROM
void ConfigPage2::DeployConfiguration()
{
    DeployCompass();
    DeployGnss();
    DeployWind();
    DeployDepth();
    DeploySpeed();

    gConfiguration.DeployConfiguration(&gMicronetDevice);
    gConfiguration.SaveToEeprom();
}

// @brief Deploy compass configuration to the overall system
void ConfigPage2::DeployCompass()
{
    switch (configCompassSel)
    {
    case 0:
        gConfiguration.eeprom.compassSource = LINK_MICRONET;
        break;
    case 1:
        gConfiguration.eeprom.compassSource = LINK_COMPASS;
        break;
    case 2:
        gConfiguration.eeprom.compassSource = LINK_NMEA_EXT;
        break;
    }
}

// @brief Deploy GNSS configuration to the overall system
void ConfigPage2::DeployGnss()
{
    switch (configGnssSel)
    {
    case 0:
        gConfiguration.eeprom.gnssSource = LINK_NMEA_GNSS;
        break;
    case 1:
        gConfiguration.eeprom.gnssSource = LINK_NMEA_EXT;
        break;
    }
}

// @brief Deploy wind configuration to the overall system
void ConfigPage2::DeployWind()
{
    switch (configWindSel)
    {
    case 0:
        gConfiguration.eeprom.windSource = LINK_MICRONET;
        break;
    case 1:
        gConfiguration.eeprom.windSource = LINK_NMEA_EXT;
        break;
    }
}

// @brief Deploy depth configuration to the overall system
void ConfigPage2::DeployDepth()
{
    switch (configDepthSel)
    {
    case 0:
        gConfiguration.eeprom.depthSource = LINK_MICRONET;
        break;
    case 1:
        gConfiguration.eeprom.depthSource = LINK_NMEA_EXT;
        break;
    }
}

// @brief Deploy speed configuration to the overall system
void ConfigPage2::DeploySpeed()
{
    switch (configSpeedSel)
    {
    case 0:
        gConfiguration.eeprom.speedSource = LINK_MICRONET;
        break;
    case 1:
        gConfiguration.eeprom.speedSource = LINK_NMEA_EXT;
        break;
    }
}
