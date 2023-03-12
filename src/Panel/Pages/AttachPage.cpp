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
    Draw the page on display
    @param force Force redraw, even if the content did not change
*/
void AttachPage::Draw(bool force, bool flushDisplay)
{
    char     lineStr[22];
    int16_t  xStr, yStr;
    uint16_t wStr, hStr;

    // Update nearest network
    nearestNetworkId             = 0;
    nearestNetworkRssi           = -10000;
    deviceInfo.nbNetworksInRange = deviceInfo.nbNetworksInRange;
    for (int i = 0; i < deviceInfo.nbNetworksInRange; i++)
    {
        deviceInfo.networksInRange[i] = deviceInfo.networksInRange[i];
        if (deviceInfo.networksInRange[i].rssi > nearestNetworkRssi)
        {
            nearestNetworkRssi = deviceInfo.networksInRange[i].rssi;
            nearestNetworkId   = deviceInfo.networksInRange[i].networkId;
        }
    }

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
            PrintCentered(2 * 8, lineStr);
            display->setTextColor(SSD1306_WHITE);
            display->fillRect(SCREEN_WIDTH - 6 * 6, 5 * 8 + (menuSelection * 8), 6 * 6, 8, SSD1306_WHITE);
            display->setTextColor((menuSelection == 0) ? SSD1306_BLACK : SSD1306_WHITE);
            PrintRight(5 * 8, "Attach");
            display->setTextColor((menuSelection == 1) ? SSD1306_BLACK : SSD1306_WHITE);
            PrintRight(6 * 8, "Exit");
        }
        else
        {
            // Panel to be displayed when no network is detected
            PrintCentered(0 * 5, "No network detected");
            display->setTextColor(SSD1306_WHITE);
            display->fillRect(128 - 6 * 4, 6 * 8, 6 * 4, 8, SSD1306_WHITE);
            display->setTextColor(SSD1306_BLACK);
            PrintRight(7 * 8, "Exit");
        }
        display->setTextColor(SSD1306_WHITE);

        if (flushDisplay)
        {
            display->display();
        }
    }
}

/*
  Function called by PanelManager when the button is pressed
  @param force Force redraw, even if the content did not change
  @param longPress true if a long press was detected
  @return Action to be executed by PanelManager
*/
PageAction_t AttachPage::OnButtonPressed(ButtonId_t buttonId, bool longPress)
{
    PageAction_t action = PAGE_ACTION_NONE;

    // Long press has not effect on this page
    if (!longPress)
    {
        // On a button 1, we execute the currently selected menu
        if (buttonId == BUTTON_ID_0)
        {
            if ((menuSelection == 0) && (nearestNetworkId != 0))
            {
                // "Attach" menu
                gConfiguration.eeprom.networkId = nearestNetworkId;
                gConfiguration.SetModifiedFlag();
            }
            else
            {
                action = PAGE_ACTION_EXIT_PAGE;
            }
        }
        else
        {
            // Button 0 : cycle through menu items
            menuSelection = (menuSelection + 1) & 0x01;
            action        = PAGE_ACTION_REFRESH;
        }
    }

    return action;
}