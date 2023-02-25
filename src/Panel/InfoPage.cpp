/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Handler of the Info page                                   *
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

#include "InfoPage.h"
#include "Globals.h"
#include "MicronetDevice.h"
#include "PanelResources.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

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

InfoPage::InfoPage()
{
}

InfoPage::~InfoPage()
{
}

/*
  Draw the page on display
  @param force Force redraw, even if the content did not change
*/
void InfoPage::Draw(bool force)
{
    char     lineStr[22];
    int16_t  xStr, yStr;
    uint16_t wStr, hStr;

    display->clearDisplay();

    display->setTextSize(1);
    display->setFont(nullptr);
    display->setTextColor(SSD1306_WHITE);

    // NetworkID
    PrintLeft(0, "NetworkID");
    if (gConfiguration.eeprom.networkId != 0)
    {
        snprintf(lineStr, sizeof(lineStr), "%08x", gConfiguration.eeprom.networkId);
        PrintRight(0, lineStr);
    }
    else
    {
        PrintRight(0, "---");
    }

    // Wind calibration
    PrintLeft(8, "Wind");
    snprintf(lineStr, sizeof(lineStr), "%.0f%% %.0f%c", gConfiguration.eeprom.windSpeedFactor_per, gConfiguration.eeprom.windDirectionOffset_deg, 247);
    PrintRight(8, lineStr);

    // Water calibration
    PrintLeft(16, "Water");
    snprintf(lineStr, sizeof(lineStr), "%.0f%% %.0fC", gConfiguration.eeprom.waterSpeedFactor_per, gConfiguration.eeprom.waterTemperatureOffset_C);
    PrintRight(16, lineStr);

    // Depth calibration
    PrintLeft(24, "Depth");
    snprintf(lineStr, sizeof(lineStr), "%.1fm", gConfiguration.eeprom.waterSpeedFactor_per);
    PrintRight(24, lineStr);

    // Heading offset
    PrintLeft(32, "Heading");
    snprintf(lineStr, sizeof(lineStr), "%.0f%c", gConfiguration.eeprom.headingOffset_deg, 247);
    PrintRight(32, lineStr);
    
    // Heading offset
    PrintLeft(40, "MagVar");
    snprintf(lineStr, sizeof(lineStr), "%.0f%c", gConfiguration.eeprom.magneticVariation_deg, 247);
    PrintRight(40, lineStr);
    
    PrintCentered(56, "Info");

    display->display();
}

// @brief Function called by PanelManager when the button is pressed
// @param longPress true if a long press was detected
// @return Action to be executed by PanelManager
PageAction_t InfoPage::OnButtonPressed(bool longPress)
{
    PageAction_t action = PAGE_ACTION_EXIT;

    if (longPress)
    {
        // Long press : do nothing
        action = PAGE_ACTION_REFRESH;
    }
    else
    {
        // Short press : cycle to next page
        action = PAGE_ACTION_EXIT;
    }

    return action;
}
