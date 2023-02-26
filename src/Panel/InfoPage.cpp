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

#define NB_SUBPAGES 3

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

InfoPage::InfoPage() : subPageIndex(0)
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
    char lineStr[22];

    display->clearDisplay();

    // Draw selected sub info page
    switch (subPageIndex)
    {
    case 0:
        DrawCalibrationInfoPage();
        break;
    case 1:
        DrawNetworkInfoPage();
        break;
    case 2:
        DrawCompassInfoPage();
        break;
    }

    snprintf(lineStr, sizeof(lineStr), "Info %d/%d", subPageIndex + 1, NB_SUBPAGES);
    PrintCentered(56, lineStr);

    display->display();
}

void InfoPage::DrawCalibrationInfoPage()
{
    char lineStr[22];

    display->setTextSize(1);
    display->setFont(nullptr);
    display->setTextColor(SSD1306_WHITE);

    // Wind calibration
    PrintLeft(0, "Wind");
    snprintf(lineStr, sizeof(lineStr), "%.0f%% %.0f%c", (gConfiguration.eeprom.windSpeedFactor_per - 1.0f) * 100.f, gConfiguration.eeprom.windDirectionOffset_deg,
             247);
    PrintRight(0, lineStr);

    // Water calibration
    PrintLeft(8, "Water");
    snprintf(lineStr, sizeof(lineStr), "%.0f%% %.0fC", (gConfiguration.eeprom.waterSpeedFactor_per - 1.0f) * 100.f, gConfiguration.eeprom.waterTemperatureOffset_C);
    PrintRight(8, lineStr);

    // Depth calibration
    PrintLeft(16, "Depth");
    snprintf(lineStr, sizeof(lineStr), "%.1fm", (gConfiguration.eeprom.waterSpeedFactor_per - 1.0f) * 100.f);
    PrintRight(16, lineStr);

    // Heading offset
    PrintLeft(24, "Heading");
    snprintf(lineStr, sizeof(lineStr), "%.0f%c", gConfiguration.eeprom.headingOffset_deg, 247);
    PrintRight(24, lineStr);

    // Heading offset
    PrintLeft(32, "MagVar");
    snprintf(lineStr, sizeof(lineStr), "%.0f%c", gConfiguration.eeprom.magneticVariation_deg, 247);
    PrintRight(32, lineStr);
}

void InfoPage::DrawNetworkInfoPage()
{
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
}

void InfoPage::DrawCompassInfoPage()
{
    char lineStr[22];

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
        subPageIndex++;
        if (subPageIndex >= NB_SUBPAGES)
        {
            subPageIndex = 0;
        }
        action = PAGE_ACTION_REFRESH;
    }
    else
    {
        // Short press : cycle to next page
        action = PAGE_ACTION_EXIT;
    }

    return action;
}
