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

#define OLED_RESET     -1 
#define SCREEN_ADDRESS 0x3C

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
        pageNumber = PAGE_NETWORK;
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
        currentPage = (PageHandler *)&networkPage;
        currentPage->SetDisplay(&display);
        currentPage->Draw();
        break;
    case PAGE_CLOCK:
        currentPage = (PageHandler *)&clockPage;
        currentPage->SetDisplay(&display);
        currentPage->Draw();
        break;
    default:
        break;
    }
}
