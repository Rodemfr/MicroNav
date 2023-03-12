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

#include "InfoPagePower.h"
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

InfoPagePower::InfoPagePower()
{
}

InfoPagePower::~InfoPagePower()
{
}

/*
  Draw the page on display
  @param force Force redraw, even if the content did not change
*/
bool InfoPagePower::Draw(bool force, bool flushDisplay)
{
    char lineStr[22];

    display->clearDisplay();

    display->setTextSize(1);
    display->setFont(nullptr);
    display->setTextColor(SSD1306_WHITE);

    PowerStatus_t powerStatus = gPower.GetStatus();

    PrintLeft(0, "BAT level");
    PrintLeft(8, "BAT");
    if (powerStatus.batteryConnected != 0)
    {
        if (powerStatus.batteryCharging)
        {
            snprintf(lineStr, sizeof(lineStr), "%c%d%%", 0x18, powerStatus.batteryLevel_per);
        }
        else
        {
            snprintf(lineStr, sizeof(lineStr), "%d%%", powerStatus.batteryLevel_per);
        }
        PrintRight(0, lineStr);
        snprintf(lineStr, sizeof(lineStr), "%.2fV/%.0fmA", powerStatus.batteryVoltage_V, powerStatus.batteryCurrent_mA);
        PrintRight(8, lineStr);
    }
    else
    {
        PrintRight(0, "---");
        PrintRight(8, "--/--");
    }

    PrintLeft(16, "USB");
    if (powerStatus.usbConnected != 0)
    {
        snprintf(lineStr, sizeof(lineStr), "%.2fV/%.0fmA", powerStatus.usbVoltage_V, powerStatus.usbCurrent_mA);
        PrintRight(16, lineStr);
    }
    else
    {
        PrintRight(16, "--/--");
    }

    PrintLeft(24, "Temp");
    snprintf(lineStr, sizeof(lineStr), "%.1f%cC", powerStatus.temperature_C, 247);
    PrintRight(24, lineStr);

    if (flushDisplay)
    {
        display->display();
    }

    return true;
}

// @brief Function called by PanelManager when the button is pressed
// @param longPress true if a long press was detected
// @return Action to be executed by PanelManager
PageAction_t InfoPagePower::OnButtonPressed(ButtonId_t buttonId, bool longPress)
{
    PageAction_t action = PAGE_ACTION_NONE;

    if ((buttonId == BUTTON_ID_1) && !longPress)
    {
        action = PAGE_ACTION_EXIT_PAGE;
    }
    else if ((buttonId == BUTTON_ID_0) && !longPress)
    {
        action = PAGE_ACTION_EXIT_TOPIC;
    }

    return action;
}
