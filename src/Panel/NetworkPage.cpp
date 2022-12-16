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
#include "MicronetDevice.h"
#include "PanelResources.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

#define DEVICE_ICON_WIDTH  32
#define DEVICE_ICON_HEIGHT 21

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

NetworkPage::NetworkPage() : deviceId(0), networkConnected(false)
{
}

NetworkPage::~NetworkPage()
{
}

void NetworkPage::Draw(bool force)
{
    static const char noNetStr[] = "No Network";
    int16_t           xStr, yStr;
    uint16_t          wStr, hStr;

    if (display != nullptr)
    {
        display->clearDisplay();
        if (networkConnected)
        {
            for (int i = 0; i < nbDevicesInRange; i++)
            {
                DrawDeviceIcon(GetIconById(devicesInRange[i].deviceId), i, (devicesInRange[i].radioLevel + 1) / 2);
            }
        }
        else
        {
            display->setTextColor(SSD1306_WHITE);
            display->setTextSize(1);
            display->setFont(&FreeSansBold9pt);
            display->getTextBounds(String(noNetStr), 0, 0, &xStr, &yStr, &wStr, &hStr);
            display->setCursor((SCREEN_WIDTH - wStr) / 2, (SCREEN_HEIGHT - yStr) / 2);
            display->println(noNetStr);
        }
        display->display();
    }
}

void NetworkPage::DrawDeviceIcon(uint8_t const *icon, uint32_t position, uint32_t radioLevel)
{
    if (position > 12)
        return;

    uint32_t xPos = DEVICE_ICON_WIDTH * (position & 0x03);
    uint32_t yPos = DEVICE_ICON_HEIGHT * (position >> 2);
    display->drawBitmap(xPos, yPos, (uint8_t *)icon, DEVICE_ICON_WIDTH, DEVICE_ICON_HEIGHT, 1);
    if (radioLevel < 5)
    {
        display->fillRect(xPos + 26, yPos + 4, 6, 3 * (5 - radioLevel), 0);
    }
}

unsigned char const *NetworkPage::GetIconById(uint32_t deviceId)
{
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

void NetworkPage::SetNetworkStatus(MicronetNetworkState_t &networkStatus)
{
    networkConnected              = networkStatus.connected;
    deviceId                      = networkStatus.deviceId;
    this->networkMap.networkId    = networkStatus.networkMap.networkId;
    this->networkMap.nbDevices    = networkStatus.networkMap.nbDevices;
    this->networkMap.masterDevice = networkStatus.networkMap.masterDevice;
    this->networkMap.nbSyncSlots  = networkStatus.networkMap.nbSyncSlots;
    for (int i = 0; i < this->networkMap.nbSyncSlots; i++)
    {
        this->networkMap.syncSlot[i] = networkStatus.networkMap.syncSlot[i];
    }
    nbDevicesInRange = networkStatus.nbDevicesInRange;
    for (int i = 0; i < nbDevicesInRange; i++)
    {
        devicesInRange[i] = networkStatus.devicesInRange[i];
    }
}
