/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Driver for T-BEAM 1.1 OLED Panel and button                   *
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

#include "BoardConfig.h"
#include "Version.h"
#include "PanelManager.h"
#include "PanelResources.h"

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

#define COMMAND_EVENT_REFRESH 0x00000001
#define COMMAND_EVENT_ALL     0x00000001

#define DISPLAY_UPDATE_PERIOD 1000

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
PanelManager* PanelManager::objectPtr;

/***************************************************************************/
/*                              Functions                                  */
/***************************************************************************/

PanelManager::PanelManager(): displayAvailable(false), pageNumber(0), currentPage((PageHandler*)&logoPage)
{
    memset(&networkStatus, 0, sizeof(networkStatus));
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

        logoPage.SetDisplay(&display);
        logoPage.SetSwversion(SW_MAJOR_VERSION, SW_MINOR_VERSION, SW_PATCH_VERSION);
        clockPage.SetDisplay(&display);
        networkPage.SetDisplay(&display);

        pageNumber = 0;

        mutex = portMUX_INITIALIZER_UNLOCKED;
        commandEventGroup = xEventGroupCreate();
        xTaskCreate(CommandProcessingTask, "DioTask", 16384, (void*)this, 5, &commandTaskHandle);
    }

    // Register button's ISR
    pinMode(BUTTON_PIN, INPUT);
    objectPtr = this;
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), ButtonHighIsr, FALLING);

    DrawPage();

    return displayAvailable;
}

void PanelManager::SetPage(uint32_t pageNumber)
{
    if ((pageNumber >= 0) && (pageNumber < PAGE_MAX_PAGES))
    {
        portENTER_CRITICAL(&mutex);
        this->pageNumber = pageNumber;
        portEXIT_CRITICAL(&mutex);
    }
}

void PanelManager::SetPageISR(uint32_t pageNumber)
{
    if ((pageNumber >= 0) && (pageNumber < PAGE_MAX_PAGES))
    {
        portENTER_CRITICAL_ISR(&mutex);
        this->pageNumber = pageNumber;
        portEXIT_CRITICAL_ISR(&mutex);
    }
}

void PanelManager::DrawPage()
{
    xEventGroupSetBits(commandEventGroup, COMMAND_EVENT_REFRESH);
}

void PanelManager::DrawPageISR()
{
    BaseType_t scheduleChange = pdFALSE;

    xEventGroupSetBitsFromISR(commandEventGroup, COMMAND_EVENT_REFRESH, &scheduleChange);
    portYIELD_FROM_ISR(scheduleChange);
}

void PanelManager::NextPage()
{
    portENTER_CRITICAL(&mutex);
    this->pageNumber = (pageNumber + 1) < PAGE_MAX_PAGES ? pageNumber + 1 : 0;
    portEXIT_CRITICAL(&mutex);
}

void PanelManager::NextPageISR()
{
    portENTER_CRITICAL_ISR(&mutex);
    this->pageNumber = (pageNumber + 1) < PAGE_MAX_PAGES ? pageNumber + 1 : 0;
    portEXIT_CRITICAL_ISR(&mutex);
}

void PanelManager::SetNavigationData(NavigationData* navData)
{
    // FIXME : Work on a copy of data instead of a direct pointer
    portENTER_CRITICAL(&mutex);
    clockPage.SetNavData(navData);
    portEXIT_CRITICAL(&mutex);
}

void PanelManager::SetNetworkStatus(MicronetNetworkState_t &networkStatus)
{
    portENTER_CRITICAL(&mutex);
    networkPage.SetNetworkStatus(networkStatus);
    logoPage.SetNetworkStatus(networkStatus);
    portEXIT_CRITICAL(&mutex);
}

void PanelManager::CommandProcessingTask(void* parameter)
{
    ((PanelManager*)parameter)->CommandCallback();
}

void PanelManager::CommandCallback()
{
    while (true)
    {
        EventBits_t commandFlags = xEventGroupWaitBits(commandEventGroup, COMMAND_EVENT_ALL, pdTRUE, pdFALSE, DISPLAY_UPDATE_PERIOD / portTICK_PERIOD_MS);

        portENTER_CRITICAL(&mutex);
        switch (pageNumber)
        {
        case PAGE_LOGO:
            currentPage = (PageHandler*)&logoPage;
            break;
        case PAGE_NETWORK:
            currentPage = (PageHandler*)&networkPage;
            break;
        case PAGE_CLOCK:
            currentPage = (PageHandler*)&clockPage;
            break;
        }
        portEXIT_CRITICAL(&mutex);

        currentPage->Draw(commandFlags & COMMAND_EVENT_REFRESH);
    }
}

void PanelManager::ButtonHighIsr()
{
    static uint32_t lastBtnHState = 0;
    uint32_t now = millis();

    if (now - lastBtnHState > 330)
    {
        lastBtnHState = now;
        objectPtr->NextPageISR();
        objectPtr->DrawPageISR();
    }
}
