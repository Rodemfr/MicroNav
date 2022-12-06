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

#include "Panel/PanelManager.h"
#include "Panel/PanelResources.h"

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT  64
#define OLED_RESET     -1 
#define SCREEN_ADDRESS 0x3C

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

PanelManager::PanelManager() : displayAvailable(false), pageNumber(0), currentPage((PageHandler *)&logoPage)
{
}

PanelManager::~PanelManager()
{
}

bool PanelManager::Init()
{
    displayAvailable = false;

    if (display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        displayAvailable = true;
        pageNumber = PAGE_LOGO;
        DrawPage();
    }

    return displayAvailable;
}

void PanelManager::SetPage(uint32_t pageNumber)
{
    if ((pageNumber >= 0) && (pageNumber < PAGE_MAX_PAGES))
    {
        this->pageNumber = pageNumber;
        DrawPage();
    }
}

void PanelManager::DrawPage()
{
    switch (pageNumber)
    {
    case PAGE_LOGO:
        currentPage = (PageHandler *)&logoPage;
        currentPage->SetDisplay(&display);
        currentPage->Draw();
        break;
    case PAGE_NETWORK:
        break;
    case PAGE_CLOCK:
        break;
    default:
        break;
    }
}

void PanelManager::DrawWelcomePage()
{
    display.clearDisplay();
    display.drawBitmap(0, 0, LOGO_BITMAP, LOGO_WIDTH, LOGO_HEIGHT, 1);
    display.display();
}

void PanelManager::DrawNetworkPage()
{
    display.clearDisplay();
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
    display.display();
}

void PanelManager::DrawClockPage()
{
    String time = "22:17";
    String date = "05/12/2022";
    int16_t xTime, yTime, xDate, yDate;
    uint16_t wTime, hTime, wDate, hDate;

    display.clearDisplay();
    display.cp437(true);
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setFont(&FreeSansBold24pt);
    display.getTextBounds(time, 0, 0, &xTime, &yTime, &wTime, &hTime);
    display.setFont(&FreeSansBold12pt);
    display.getTextBounds(date, 0, 0, &xDate, &yDate, &wDate, &hDate);

    display.setFont(&FreeSansBold24pt);
    display.setCursor((SCREEN_WIDTH - wTime) / 2, -yTime + (SCREEN_HEIGHT + yTime + yDate - 6) / 2);
    display.println(time);

    display.setFont(&FreeSansBold12pt);
    display.setCursor((SCREEN_WIDTH - wDate) / 2, -yTime - yDate + 6 + (SCREEN_HEIGHT + yTime + yDate - 6) / 2);
    display.println(date);
    display.display();
}

void PanelManager::DrawDeviceIcon(uint8_t const* icon, uint32_t position, uint32_t radioLevel)
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
