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

#include "InfoPageCompass.h"
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

InfoPageCompass::InfoPageCompass()
{
}

InfoPageCompass::~InfoPageCompass()
{
}

/*
  Draw the page on display
  @param force Force redraw, even if the content did not change
*/
bool InfoPageCompass::Draw(bool force, bool flushDisplay)
{
    char lineStr[22];

    display->clearDisplay();

    display->setTextSize(1);
    display->setFont(nullptr);
    display->setTextColor(SSD1306_WHITE);

    // Compass connected ?
    PrintLeft(0, "NavCompass");
    if (gConfiguration.ram.navCompassAvailable != 0)
    {
        PrintRight(0, "LSM303");
    }
    else
    {
        PrintRight(0, "No");
    }

    // X axis offset
    PrintLeft(8, "X offset");
    snprintf(lineStr, sizeof(lineStr), "%.1f%", gConfiguration.eeprom.xMagOffset);
    PrintRight(8, lineStr);

    // Y axis offset
    PrintLeft(16, "Y offset");
    snprintf(lineStr, sizeof(lineStr), "%.1f%", gConfiguration.eeprom.yMagOffset);
    PrintRight(16, lineStr);

    // Z axis offset
    PrintLeft(24, "Z offset");
    snprintf(lineStr, sizeof(lineStr), "%.1f%", gConfiguration.eeprom.zMagOffset);
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
PageAction_t InfoPageCompass::OnButtonPressed(ButtonId_t buttonId, bool longPress)
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
