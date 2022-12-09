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

#define COMMAND_EVENT_REFRESH         0x00000001
#define COMMAND_EVENT_BUTTON_RELEASED 0x00000002
#define COMMAND_EVENT_ALL             0x00000003

#define TASK_WAKEUP_PERIOD      200
#define DISPLAY_UPDATE_PERIOD   1000
#define BUTTON_LONG_PRESS_DELAY 1500
#define BUTTON_SHORT_PRESS_MIN  250

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
        configPage.SetDisplay(&display);

        pageNumber = 0;

        commandMutex = portMUX_INITIALIZER_UNLOCKED;
        buttonMutex = portMUX_INITIALIZER_UNLOCKED;
        commandEventGroup = xEventGroupCreate();
        xTaskCreate(CommandProcessingTask, "DioTask", 16384, (void*)this, 5, &commandTaskHandle);
    }

    // Register button's ISR
    pinMode(BUTTON_PIN, INPUT);
    objectPtr = this;
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), StaticButtonIsr, CHANGE);

    DrawPage();

    return displayAvailable;
}

void PanelManager::SetPage(uint32_t pageNumber)
{
    if ((pageNumber >= 0) && (pageNumber < PAGE_MAX_PAGES))
    {
        portENTER_CRITICAL(&commandMutex);
        this->pageNumber = pageNumber;
        portEXIT_CRITICAL(&commandMutex);
    }
}

void PanelManager::SetPageISR(uint32_t pageNumber)
{
    if ((pageNumber >= 0) && (pageNumber < PAGE_MAX_PAGES))
    {
        portENTER_CRITICAL_ISR(&commandMutex);
        this->pageNumber = pageNumber;
        portEXIT_CRITICAL_ISR(&commandMutex);
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
    portENTER_CRITICAL(&commandMutex);
    this->pageNumber = (pageNumber + 1) < PAGE_MAX_PAGES ? pageNumber + 1 : 0;
    portEXIT_CRITICAL(&commandMutex);
}

void PanelManager::NextPageISR()
{
    portENTER_CRITICAL_ISR(&commandMutex);
    this->pageNumber = (pageNumber + 1) < PAGE_MAX_PAGES ? pageNumber + 1 : 0;
    portEXIT_CRITICAL_ISR(&commandMutex);
}

void PanelManager::SetNavigationData(NavigationData* navData)
{
    // FIXME : Work on a copy of data instead of a direct pointer
    portENTER_CRITICAL(&commandMutex);
    clockPage.SetNavData(navData);
    portEXIT_CRITICAL(&commandMutex);
}

void PanelManager::SetNetworkStatus(MicronetNetworkState_t& networkStatus)
{
    portENTER_CRITICAL(&commandMutex);
    networkPage.SetNetworkStatus(networkStatus);
    portEXIT_CRITICAL(&commandMutex);
}

void PanelManager::CommandProcessingTask(void* parameter)
{
    ((PanelManager*)parameter)->CommandCallback();
}

void PanelManager::CommandCallback()
{
    uint32_t lastPageUpdate = 0;
    uint32_t now;
    uint32_t localLastPress;
    uint32_t localLastRelease;
    bool localButtonPressed;
    bool longPress;

    while (true)
    {
        EventBits_t commandFlags = xEventGroupWaitBits(commandEventGroup, COMMAND_EVENT_ALL, pdTRUE, pdFALSE, TASK_WAKEUP_PERIOD / portTICK_PERIOD_MS);
        now = millis();

        portENTER_CRITICAL(&buttonMutex);
        localLastPress = lastPress;
        localLastRelease = lastRelease;
        localButtonPressed = buttonPressed;
        portEXIT_CRITICAL(&buttonMutex);

        if ((localButtonPressed == true) && ((now - localLastPress) > BUTTON_LONG_PRESS_DELAY) && !longPress)
        {
            longPress = true;
            switch (currentPage->OnButtonPressed(true))
            {
            case PAGE_ACTION_NEXT_PAGE:
                NextPage();
                commandFlags |= COMMAND_EVENT_REFRESH;
                break;
            case PAGE_ACTION_REFRESH:
                commandFlags |= COMMAND_EVENT_REFRESH;
                break;
            }
        }

        if ((commandFlags & COMMAND_EVENT_BUTTON_RELEASED) && (localButtonPressed == false) &&
            ((localLastRelease - lastPageUpdate) > BUTTON_SHORT_PRESS_MIN))
        {
            if (longPress)
            {
                longPress = false;
            }
            else
            {
                // Short press
                switch (currentPage->OnButtonPressed(false))
                {
                case PAGE_ACTION_NEXT_PAGE:
                    NextPage();
                    commandFlags |= COMMAND_EVENT_REFRESH;
                    break;
                case PAGE_ACTION_REFRESH:
                    commandFlags |= COMMAND_EVENT_REFRESH;
                    break;
                }
            }
        }

        if ((commandFlags & COMMAND_EVENT_REFRESH) || ((now - lastPageUpdate) > DISPLAY_UPDATE_PERIOD))
        {
            lastPageUpdate = now;
            portENTER_CRITICAL(&commandMutex);
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
            case PAGE_CONFIG:
                currentPage = (PageHandler*)&configPage;
                break;
            }
            portEXIT_CRITICAL(&commandMutex);

            currentPage->Draw(commandFlags & COMMAND_EVENT_REFRESH);
        }
    }
}

void PanelManager::StaticButtonIsr()
{
    objectPtr->ButtonIsr();
}

void PanelManager::ButtonIsr()
{
    uint32_t now = millis();
    BaseType_t scheduleChange = pdFALSE;

    if (!digitalRead(BUTTON_PIN))
    {
        // Button pressed
        portENTER_CRITICAL_ISR(&buttonMutex);
        buttonPressed = true;
        lastPress = now;
        portEXIT_CRITICAL_ISR(&buttonMutex);
    }
    else
    {
        // Button released
        portENTER_CRITICAL_ISR(&buttonMutex);
        buttonPressed = false;
        lastRelease = now;
        portEXIT_CRITICAL_ISR(&buttonMutex);

        xEventGroupSetBitsFromISR(commandEventGroup, COMMAND_EVENT_BUTTON_RELEASED, &scheduleChange);
        portYIELD_FROM_ISR(scheduleChange);
    }
}
