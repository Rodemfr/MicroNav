/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Handler of the Network Attachment page                        *
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

#include "AttachPage.h"
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

AttachPage::AttachPage() : menuSelection(0), nearestNetworkId(0), nearestNetworkRssi(0)
{
}

AttachPage::~AttachPage()
{
}

/*
  Set the latest network status.
  @param deviceInfo Structure
*/
void AttachPage::SetNetworkStatus(DeviceInfo_t &deviceInfo)
{
    nearestNetworkId   = 0;
    nearestNetworkRssi = -10000;

    nbNetworksInRange = deviceInfo.nbNetworksInRange;
    for (int i = 0; i < nbNetworksInRange; i++)
    {
        networksInRange[i] = deviceInfo.networksInRange[i];
        if (networksInRange[i].rssi > nearestNetworkRssi)
        {
            nearestNetworkRssi = networksInRange[i].rssi;
            nearestNetworkId   = networksInRange[i].networkId;
        }
    }
}

/*
  Draw the page on display
  @param force Force redraw, even if the content did not change
*/
void AttachPage::Draw(bool force)
{
    char     lineStr[22];
    int16_t  xStr, yStr;
    uint16_t wStr, hStr;

    if (display != nullptr)
    {
        display->clearDisplay();

        // Config items
        display->setTextSize(1);
        display->setFont(nullptr);
        display->setTextColor(SSD1306_WHITE);

        if (nearestNetworkId != 0)
        {
            // Panel to be display when a network is detected
            PrintCentered(0 * 8, "Nearest network");
            snprintf(lineStr, sizeof(lineStr), "%0x", nearestNetworkId);
            PrintCentered(1 * 8, lineStr);
            display->setTextColor(SSD1306_WHITE);
            display->fillRect(0 + (menuSelection * SCREEN_WIDTH / 2), 7 * 8, SCREEN_WIDTH / 2, 8, SSD1306_WHITE);
            display->setTextColor((menuSelection == 0) ? SSD1306_BLACK : SSD1306_WHITE);
            PrintCentered(SCREEN_WIDTH / 4, 7 * 8, "Attach");
            display->setTextColor((menuSelection == 1) ? SSD1306_BLACK : SSD1306_WHITE);
            PrintCentered(3 * SCREEN_WIDTH / 4, 7 * 8, "Exit");
        }
        else
        {
            // Panel to be displayed when no network is detected
            PrintCentered(0 * 8, "No network detected");
            display->setTextColor(SSD1306_WHITE);
            display->fillRect(SCREEN_WIDTH / 4, 7 * 8, SCREEN_WIDTH / 2, 8, SSD1306_WHITE);
            display->setTextColor(SSD1306_BLACK);
            PrintCentered(SCREEN_WIDTH / 2, 7 * 8, "Exit");
        }
        display->setTextColor(SSD1306_WHITE);

        display->display();
    }
}

/*
  Function called by PanelManager when the button is pressed
  @param force Force redraw, even if the content did not change
  @param longPress true if a long press was detected
  @return Action to be executed by PanelManager
*/
PageAction_t AttachPage::OnButtonPressed(bool longPress)
{
    PageAction_t action = PAGE_ACTION_EXIT;

    // On a long press, we execute the currently selected menu
    if (longPress)
    {
        if ((menuSelection == 0) && (nearestNetworkId != 0))
        {
            // Long press on "Attach"
            gConfiguration.eeprom.networkId = nearestNetworkId;
        }
    }
    else
    {
        // Short press : cycle through menu items
        menuSelection = (menuSelection + 1) & 0x01;
        action        = PAGE_ACTION_REFRESH;
    }

    return action;
}