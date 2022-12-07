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

NetworkPage::NetworkPage()
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
        DrawDeviceIcon(T000_BITMAP, 0, 5);
        DrawDeviceIcon(T110_BITMAP, 1, 4);
        DrawDeviceIcon(T111_BITMAP, 2, 5);
        DrawDeviceIcon(T112_BITMAP, 3, 4);
        DrawDeviceIcon(T113_BITMAP, 4, 5);
        DrawDeviceIcon(T120_BITMAP, 5, 4);
        DrawDeviceIcon(T121_BITMAP, 6, 5);
        DrawDeviceIcon(T210_BITMAP, 7, 4);
        DrawDeviceIcon(T215_BITMAP, 8, 5);
        DrawDeviceIcon(T060_BITMAP, 9, 4);
        DrawDeviceIcon(T075_BITMAP, 10, 5);
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
