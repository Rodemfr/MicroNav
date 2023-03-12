/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Handler of the Network page                                   *
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

#include "NetworkPage.h"
#include "Globals.h"
#include "MicronetDevice.h"
#include "PanelResources.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

#define DEVICE_ICON_WIDTH  32 // Width of device icons
#define DEVICE_ICON_HEIGHT 21 // Height of device icons
#define NB_ICONS_PER_PAGE  8  // Maximum number of icons that can be displayed on a page

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

/*
  Class constructor
*/
NetworkPage::NetworkPage()
{
}

/*
  Class destructor
*/
NetworkPage::~NetworkPage()
{
}

/*
  Draw the page on display panel
  @param force Force complete redraw of the page, even if there are no changes.
*/
void NetworkPage::Draw(bool force, bool flushDisplay)
{
    static const char noNetStr[] = "No Network";
    int16_t           xStr, yStr;
    uint16_t          wStr, hStr;
    uint32_t          localRadioLevel;
    uint32_t          remoteRadioLevel;
    char              lineStr[32];

    if (display != nullptr)
    {
        // Clear the display
        display->clearDisplay();
        // Check if the network is connected
        if (deviceInfo.state == DEVICE_STATE_ACTIVE)
        {
            // Yes : draw icons of each device
            for (int i = 0; i < deviceInfo.nbDevicesInRange; i++)
            {
                localRadioLevel  = deviceInfo.devicesInRange[i].localRadioLevel;
                remoteRadioLevel = deviceInfo.devicesInRange[i].remoteRadioLevel;
                if (deviceInfo.networkMap.masterDevice == deviceInfo.devicesInRange[i].deviceId)
                {
                    // If the device is the master device, force its remote reception level to maximum
                    remoteRadioLevel = 9;
                }
                DrawDeviceIcon(GetIconById(deviceInfo.devicesInRange[i].deviceId), i, localRadioLevel, remoteRadioLevel);
            }
        }
        else
        {
            // No : write status text
            display->setTextColor(SSD1306_WHITE);
            display->setTextSize(1);
            display->setFont(&FreeSansBold9pt);
            display->getTextBounds(String(noNetStr), 0, 0, &xStr, &yStr, &wStr, &hStr);
            display->setCursor((SCREEN_WIDTH - wStr) / 2, (SCREEN_HEIGHT - 8 - yStr) / 2 - 8);
            display->println(noNetStr);
        }

        // Send rendered buffer to display
        if (flushDisplay)
        {
            display->display();
        }
    }
}

/*
  Draw the icon of one device
  @param icon Pointer to the icon to draw
  @param position Poisiton of the device in the network
  @param localRadioLevel Signal strength of the device as seen by MicroNav
  @param remoteRadioLevel Signal strength of the network as seen by the micronet device
*/
void NetworkPage::DrawDeviceIcon(uint8_t const *icon, uint32_t position, uint32_t localRadioLevel, uint32_t remoteRadioLevel)
{
    // Only NB_ICONS_PER_PAGE icons can be displayed on one page
    if (position >= NB_ICONS_PER_PAGE)
        return;

    // Convert levels to 5-steps values
    localRadioLevel  = (localRadioLevel + 1) / 2;
    remoteRadioLevel = (remoteRadioLevel + 1) / 2;

    uint32_t xPos = DEVICE_ICON_WIDTH * (position & 0x03);
    uint32_t yPos = DEVICE_ICON_HEIGHT * (position >> 2);
    display->drawBitmap(xPos, yPos, (uint8_t *)icon, DEVICE_ICON_WIDTH, DEVICE_ICON_HEIGHT, 1);
    if (localRadioLevel < 5)
    {
        display->fillRect(xPos + 30, yPos + 4, 2, 3 * (5 - localRadioLevel), 0);
    }
    if (remoteRadioLevel < 5)
    {
        display->fillRect(xPos + 27, yPos + 4, 2, 3 * (5 - remoteRadioLevel), 0);
    }
}

/*
  Get a pointer to the icon corresponding to the provided device ID.
  @param deviceId ID of the device to get the icon for
  @return Pointer to the icon bitmap
*/
unsigned char const *NetworkPage::GetIconById(uint32_t deviceId)
{
    // Default icon to used if device ID is not identified
    unsigned char const *bitmapPtr = T000_BITMAP;

    switch (deviceId >> 24)
    {
    case MICRONET_DEVICE_TYPE_HULL_TRANSMITTER:
        bitmapPtr = T121_BITMAP;
        break;
    case MICRONET_DEVICE_TYPE_WIND_TRANSDUCER:
        bitmapPtr = T120_BITMAP;
        break;
    case MICRONET_DEVICE_TYPE_NMEA_CONVERTER:
        bitmapPtr = T000_BITMAP;
        break;
    case MICRONET_DEVICE_TYPE_MAST_ROTATION:
        bitmapPtr = T000_BITMAP;
        break;
    case MICRONET_DEVICE_TYPE_MOB:
        bitmapPtr = T000_BITMAP;
        break;
    case MICRONET_DEVICE_TYPE_SDPOD:
        bitmapPtr = T000_BITMAP;
        break;
    case MICRONET_DEVICE_TYPE_DUAL_DISPLAY:
        bitmapPtr = T111_BITMAP;
        break;
    case MICRONET_DEVICE_TYPE_ANALOG_WIND_DISPLAY:
        bitmapPtr = T112_BITMAP;
        break;
    case MICRONET_DEVICE_TYPE_DUAL_MAXI_DISPLAY:
        bitmapPtr = T215_BITMAP;
        break;
    case MICRONET_DEVICE_TYPE_REMOTE_DISPLAY:
        bitmapPtr = T210_BITMAP;
        break;
    }

    return bitmapPtr;
}
