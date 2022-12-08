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
#include "PanelResources.h"
#include "MicronetDevice.h"

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

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

NetworkPage::NetworkPage(): deviceId(0)
{
}

NetworkPage::~NetworkPage()
{
}

void NetworkPage::Draw(bool force)
{
    if (display != nullptr)
    {
        display->clearDisplay();
        DrawDeviceIcon(GetIconById(networkMap.masterDevice), 0, 5);
        for (int i = 0; i < networkMap.nbSyncSlots; i++)
        {
            if ((networkMap.syncSlot[i].deviceId >> 24) != MICRONET_DEVICE_TYPE_NMEA_CONVERTER)
            {
                DrawDeviceIcon(GetIconById(networkMap.syncSlot[i].deviceId), i + 1, 5);
            }
        }
        display->display();
    }
}

void NetworkPage::DrawDeviceIcon(uint8_t const* icon, uint32_t position, uint32_t radioLevel)
{
    if (position > 12)
        return;

    uint32_t xPos = DEVICE_ICON_WIDTH * (position & 0x03);
    uint32_t yPos = DEVICE_ICON_HEIGHT * (position >> 2);
    display->drawBitmap(xPos, yPos, (uint8_t*)icon, DEVICE_ICON_WIDTH, DEVICE_ICON_HEIGHT, 1);
    if (radioLevel < 5)
    {
        display->fillRect(xPos + 26, yPos + 4, 6, 3 * (5 - radioLevel), 0);
    }
}

unsigned char const* NetworkPage::GetIconById(uint32_t deviceId)
{
    unsigned char const* bitmapPtr = T000_BITMAP;

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

    if (bitmapPtr == T000_BITMAP)
    {
        Serial.println(deviceId, HEX);
    }

    return bitmapPtr;
}

void NetworkPage::SetNetworkStatus(MicronetNetworkState_t& networkStatus)
{
    deviceId = networkStatus.deviceId;
    this->networkMap.networkId = networkStatus.networkMap.networkId;
    this->networkMap.nbDevices = networkStatus.networkMap.nbDevices;
    this->networkMap.masterDevice = networkStatus.networkMap.masterDevice;
    this->networkMap.nbSyncSlots = networkStatus.networkMap.nbSyncSlots;
    for (int i = 0; i < this->networkMap.nbSyncSlots; i++)
    {
        this->networkMap.syncSlot[i] = networkStatus.networkMap.syncSlot[i];
    }
}
