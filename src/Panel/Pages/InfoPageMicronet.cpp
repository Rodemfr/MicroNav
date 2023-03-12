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

#include "InfoPageMicronet.h"
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

InfoPageMicronet::InfoPageMicronet()
{
}

InfoPageMicronet::~InfoPageMicronet()
{
}

/*
  Draw the page on display
  @param force Force redraw, even if the content did not change
*/
void InfoPageMicronet::Draw(bool force, bool flushDisplay)
{
    display->clearDisplay();

    char lineStr[22];

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

    // DeviceID
    PrintLeft(8, "DeviceID");
    snprintf(lineStr, sizeof(lineStr), "%08x", gConfiguration.eeprom.deviceId);
    PrintRight(8, lineStr);

    // Network status
    PrintLeft(16, "Connected");
    if (deviceInfo.state == DEVICE_STATE_ACTIVE)
    {
        PrintRight(16, "YES");
    }
    else
    {
        PrintRight(16, "NO");
    }

    // Number of devices
    PrintLeft(24, "Nb devices");
    if (deviceInfo.state == DEVICE_STATE_ACTIVE)
    {
        snprintf(lineStr, sizeof(lineStr), "%d", deviceInfo.nbDevicesInRange);
        PrintRight(24, lineStr);
    }
    else
    {
        PrintRight(24, "---");
    }

    // Other networks in range
    PrintLeft(32, "Networks in range");
    snprintf(lineStr, sizeof(lineStr), "%d", deviceInfo.nbNetworksInRange);
    PrintRight(32, lineStr);

    if (flushDisplay)
    {
        display->display();
    }
}

// @brief Function called by PanelManager when the button is pressed
// @param longPress true if a long press was detected
// @return Action to be executed by PanelManager
PageAction_t InfoPageMicronet::OnButtonPressed(ButtonId_t buttonId, bool longPress)
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
