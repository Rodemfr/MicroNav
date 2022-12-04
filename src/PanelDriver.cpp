/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Driver for T-BEAM 1.1 OLED Panel                              *
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

#include "PanelDriver.h"

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "logo.h"
#include "t000.h"
#include "t060.h"
#include "t075.h"
#include "t110.h"
#include "t111.h"
#include "t112.h"
#include "t113.h"
#include "t120.h"
#include "t121.h"
#include "t210.h"
#include "t215.h"

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

#define SCREEN_WIDTH   128 // OLED display width, in pixels
#define SCREEN_HEIGHT  64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

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

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/***************************************************************************/
/*                              Functions                                  */
/***************************************************************************/

PanelDriver::PanelDriver() : displayAvailable(false), pageNumber(0)
{
}

PanelDriver::~PanelDriver()
{
}

bool PanelDriver::Init()
{
    displayAvailable = false;

    if (display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        displayAvailable = true;
        pageNumber = 1;
        DrawPage();
    }

    return displayAvailable;
}

void PanelDriver::SetPage(uint32_t pageNumber)
{
    if ((pageNumber >= 0) && (pageNumber < PAGE_MAX_PAGES))
    {
        this->pageNumber = pageNumber;
        DrawPage();
    }
}

void PanelDriver::DrawPage()
{
    switch (pageNumber)
    {
    case PAGE_WELCOME:
        DrawWelcomePage();
        break;
    case PAGE_NETWORK:
        DrawNetworkPage();
        break;
    default:
        break;
    }
}

void PanelDriver::DrawWelcomePage()
{
    display.clearDisplay();
    display.drawBitmap(0, 0,
        LOGO,
        LOGO_WIDTH,
        LOGO_HEIGHT,
        1);
    display.display();
}

void PanelDriver::DrawNetworkPage()
{
    display.clearDisplay();
    DrawDeviceIcon(T000, 0, 0);
    DrawDeviceIcon(T110, 1, 1);
    DrawDeviceIcon(T111, 2, 2);
    DrawDeviceIcon(T112, 3, 3);
    DrawDeviceIcon(T113, 4, 5);
    DrawDeviceIcon(T120, 5, 4);
    DrawDeviceIcon(T121, 6, 3);
    DrawDeviceIcon(T210, 7, 2);
    DrawDeviceIcon(T215, 8, 1);
    DrawDeviceIcon(T060, 9, 0);
    DrawDeviceIcon(T075, 10, 5);
    display.display();
}

void PanelDriver::DrawDeviceIcon(uint8_t const* icon, uint32_t position, uint32_t radioLevel)
{
    if (position > 12)
        return;

    uint32_t xPos = DEVICE_ICON_WIDTH * (position & 0x03);
    uint32_t yPos = DEVICE_ICON_HEIGHT * (position >> 2);
    display.drawBitmap(xPos, yPos, (uint8_t*)icon, DEVICE_ICON_WIDTH, DEVICE_ICON_HEIGHT, 1);
    if (radioLevel < 5)
    {
        display.fillRect(xPos + 26, yPos + 4, 6, 3 * (5 - radioLevel), 0);
    }
}
