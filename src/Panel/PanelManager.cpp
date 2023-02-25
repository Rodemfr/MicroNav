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

#include "PanelManager.h"
#include "BoardConfig.h"
#include "PanelResources.h"
#include "Version.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

#define OLED_RESET     -1   // Pin controlling OLED reset (-1 for none)
#define SCREEN_ADDRESS 0x3C // I2C address of the display controller

#define COMMAND_EVENT_REFRESH         0x00000001 // Command flag requesting a page refresh/redraw
#define COMMAND_EVENT_BUTTON_RELEASED 0x00000002 // Command flag signaling that the button has been released
#define COMMAND_EVENT_NEW_PAGE        0x00000004 // Command flag requesting to select the next page
#define COMMAND_EVENT_ALL             0x00000007 // Command mask

#define TASK_WAKEUP_PERIOD_MS   200  // Wake-up period of the command task in milliseconds
#define DISPLAY_UPDATE_PERIOD   1000 // Display update/refresh period in milliseconds
#define BUTTON_LONG_PRESS_DELAY 1000 // Minimum time required to trigger a long button press
#define BUTTON_SHORT_PRESS_MIN  200  // Minimum time to trigger a short button press

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
PanelManager    *PanelManager::objectPtr;

/***************************************************************************/
/*                              Functions                                  */
/***************************************************************************/

/*
  Constructor of the PanelManager class instance
*/
PanelManager::PanelManager() : displayAvailable(false), pageNumber(0), currentPage((PageHandler *)&logoPage)
{
    memset(&networkStatus, 0, sizeof(networkStatus));
}

/*
  Destructor of the PanelManager class instance
*/
PanelManager::~PanelManager()
{
}

/*
  Initialize PanelManager.
  @return true if initialization is successful
*/
bool PanelManager::Init()
{
    displayAvailable = false;

    if (display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        displayAvailable = true;

        PageHandler::SetDisplay(&display);
        logoPage.SetSwversion(SW_MAJOR_VERSION, SW_MINOR_VERSION, SW_PATCH_VERSION);

        pageNumber = 0;

        commandMutex      = portMUX_INITIALIZER_UNLOCKED;
        buttonMutex       = portMUX_INITIALIZER_UNLOCKED;
        commandEventGroup = xEventGroupCreate();
        xTaskCreate(CommandProcessingTask, "DioTask", 16384, (void *)this, 5, &commandTaskHandle);
    }

    // Register button's ISR
    pinMode(BUTTON_PIN, INPUT);
    objectPtr = this;
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), StaticButtonIsr, CHANGE);

    DrawPage();

    return displayAvailable;
}

/*
  Set the page to be displayed
  @param pageNumber Index of the page as per PanelPages_t enum definition
*/
void PanelManager::SetPage(uint32_t pageNumber)
{
    if ((pageNumber >= 0) && (pageNumber < PAGE_MAX_PAGES))
    {
        portENTER_CRITICAL(&commandMutex);
        this->pageNumber = pageNumber;
        portEXIT_CRITICAL(&commandMutex);
    }
}

/*
  Set the page to be displayed (callable from ISR)
  @param pageNumber Index of the page as per PanelPages_t enum definition
*/
void PanelManager::SetPageISR(uint32_t pageNumber)
{
    if ((pageNumber >= 0) && (pageNumber < PAGE_MAX_PAGES))
    {
        portENTER_CRITICAL_ISR(&commandMutex);
        this->pageNumber = pageNumber;
        portEXIT_CRITICAL_ISR(&commandMutex);
    }
}

/*
  Request redraw of the current page
*/
void PanelManager::DrawPage()
{
    xEventGroupSetBits(commandEventGroup, COMMAND_EVENT_REFRESH);
}

/*
  Request redraw of the current page (callable from ISR)
*/
void PanelManager::DrawPageISR()
{
    BaseType_t scheduleChange = pdFALSE;

    xEventGroupSetBitsFromISR(commandEventGroup, COMMAND_EVENT_REFRESH, &scheduleChange);
    portYIELD_FROM_ISR(scheduleChange);
}

/*
  Request change of the current page
*/
void PanelManager::NextPage()
{
    portENTER_CRITICAL(&commandMutex);
    this->pageNumber = (pageNumber + 1) < PAGE_MAX_PAGES ? pageNumber + 1 : 0;
    portEXIT_CRITICAL(&commandMutex);
}

/*
  Request change of the current page (callable from ISR)
*/
void PanelManager::NextPageISR()
{
    portENTER_CRITICAL_ISR(&commandMutex);
    this->pageNumber = (pageNumber + 1) < PAGE_MAX_PAGES ? pageNumber + 1 : 0;
    portEXIT_CRITICAL_ISR(&commandMutex);
}

/*
  Gives PanelManager the latest version of navigation data for pages which needs it
  @param navData Pointer to the latest navigation dataset
*/
void PanelManager::SetNavigationData(NavigationData *navData)
{
    // FIXME : Work on a copy of data instead of a direct pointer
    portENTER_CRITICAL(&commandMutex);
    clockPage.SetNavData(navData);
    portEXIT_CRITICAL(&commandMutex);
}

/*
  Gives PanelManager the latest status of the network connection.
  The structure will be copied internally and doesn't need to be kept allocated by the caller
  @param networkStatus Pointer to the latest network status
*/
void PanelManager::SetNetworkStatus(DeviceInfo_t &networkStatus)
{
    portENTER_CRITICAL(&commandMutex);
    PageHandler::SetNetworkStatus(networkStatus);
    portEXIT_CRITICAL(&commandMutex);
}

/*
  Static entry point of the command processing task
  @param callingObject Pointer to the calling PanelManager instance
*/
void PanelManager::CommandProcessingTask(void *callingObject)
{
    // Task entry points are static -> switch to non static processing method
    ((PanelManager *)callingObject)->CommandCallback();
}

/*
  Non-static implementation of the command processing task
*/
void PanelManager::CommandCallback()
{
    uint32_t lastPageUpdate = 0;
    uint32_t now;
    uint32_t localLastPress;
    uint32_t localLastRelease;
    bool     localButtonPressed;
    bool     longPress;

    while (true)
    {
        // Wait for the next command
        EventBits_t commandFlags =
            xEventGroupWaitBits(commandEventGroup, COMMAND_EVENT_ALL, pdTRUE, pdFALSE, TASK_WAKEUP_PERIOD_MS / portTICK_PERIOD_MS);
        now = millis();

        // Collect latest status of press button
        portENTER_CRITICAL(&buttonMutex);
        localLastPress     = lastPress;
        localLastRelease   = lastRelease;
        localButtonPressed = buttonPressed;
        portEXIT_CRITICAL(&buttonMutex);

        // Do we have a long press on the button ?
        if ((localButtonPressed == true) && ((now - localLastPress) > BUTTON_LONG_PRESS_DELAY) && !longPress)
        {
            // Button is pressed for more than the long press time : handle long press processing
            longPress = true;
            // Request action to the currently displayed page
            switch (currentPage->OnButtonPressed(true))
            {
            case PAGE_ACTION_EXIT:
                // Go to next page
                NextPage();
                commandFlags |= COMMAND_EVENT_NEW_PAGE;
                break;
            case PAGE_ACTION_REFRESH:
                // Refresh current page
                commandFlags |= COMMAND_EVENT_REFRESH;
                break;
            }
        }

        // Has button been released ?
        if ((commandFlags & COMMAND_EVENT_BUTTON_RELEASED) && (localButtonPressed == false))
        {
            // Are we exiting a long press ?
            if (longPress)
            {
                // Yes : stop long press and don't check for short press
                longPress = false;
            }
            // Short press : is the press time above minimum time (anti rebound) ?
            else if ((localLastRelease - lastPageUpdate) > BUTTON_SHORT_PRESS_MIN)
            {
                // Yes : we have a short press
                // Request action to the currently displayed page
                switch (currentPage->OnButtonPressed(false))
                {
                case PAGE_ACTION_EXIT:
                    // Go to next page
                    NextPage();
                    commandFlags |= COMMAND_EVENT_NEW_PAGE;
                    break;
                case PAGE_ACTION_REFRESH:
                    // Refresh current page
                    commandFlags |= COMMAND_EVENT_REFRESH;
                    break;
                }
            }
        }

        // Process page change/refresh if requested
        if ((commandFlags & COMMAND_EVENT_REFRESH) || (commandFlags & COMMAND_EVENT_NEW_PAGE) || ((now - lastPageUpdate) > DISPLAY_UPDATE_PERIOD))
        {
            lastPageUpdate = now;
            portENTER_CRITICAL(&commandMutex);
            switch (pageNumber)
            {
            case PAGE_LOGO:
                currentPage = (PageHandler *)&logoPage;
                break;
            case PAGE_NETWORK:
                currentPage = (PageHandler *)&networkPage;
                break;
            case PAGE_INFO:
                currentPage = (PageHandler *)&infoPage;
                break;
            case PAGE_CLOCK:
                currentPage = (PageHandler *)&clockPage;
                break;
            case PAGE_CONFIG1:
                currentPage = (PageHandler *)&configPage1;
                break;
            case PAGE_CONFIG2:
                currentPage = (PageHandler *)&configPage2;
                break;
            case PAGE_COMMAND:
                currentPage = (PageHandler *)&commandPage;
                break;
            }
            portEXIT_CRITICAL(&commandMutex);

            currentPage->Draw(commandFlags & COMMAND_EVENT_NEW_PAGE);
        }
    }
}

/*
  Static entry point of the button ISR
*/
void IRAM_ATTR PanelManager::StaticButtonIsr()
{
    // Call non-static implementation of the ISR
    objectPtr->ButtonIsr();
}

/*
  Non static implementation of button ISR
*/
void PanelManager::ButtonIsr()
{
    uint32_t   now            = millis();
    BaseType_t scheduleChange = pdFALSE;

    if (!digitalRead(BUTTON_PIN))
    {
        // Button pressed
        portENTER_CRITICAL_ISR(&buttonMutex);
        buttonPressed = true;
        lastPress     = now;
        portEXIT_CRITICAL_ISR(&buttonMutex);
    }
    else
    {
        // Button released
        portENTER_CRITICAL_ISR(&buttonMutex);
        buttonPressed = false;
        lastRelease   = now;
        portEXIT_CRITICAL_ISR(&buttonMutex);

        xEventGroupSetBitsFromISR(commandEventGroup, COMMAND_EVENT_BUTTON_RELEASED, &scheduleChange);
        portYIELD_FROM_ISR(scheduleChange);
    }
}

/*
  Enable/disable panel low power mode
*/
void PanelManager::LowPower(bool enable)
{
    display.dim(enable);
}
