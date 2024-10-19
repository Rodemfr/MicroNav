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

#define COMMAND_EVENT_REFRESH            0x00000001 // Command flag requesting a page refresh/redraw
#define COMMAND_EVENT_BUTTON_RELEASED    0x00000002 // Command flag signaling that the button has been released
#define COMMAND_EVENT_NEW_PAGE           0x00000004 // Command flag requesting to select the next page
#define COMMAND_EVENT_POWER_BUTTON_SHORT 0x00000008 // Command flag signaling that the power button has been short pressed
#define COMMAND_EVENT_POWER_BUTTON_LONG  0x00000010 // Command flag signaling that the power button has been short pressed
#define COMMAND_EVENT_ALL                0x0000001F // Command mask

#define TASK_WAKEUP_PERIOD_MS   200  // Wake-up period of the command task in milliseconds
#define DISPLAY_UPDATE_PERIOD   1000 // Display update/refresh period in milliseconds
#define BUTTON_LONG_PRESS_DELAY 1000 // Minimum time required to trigger a long button press
#define BUTTON_SHORT_PRESS_MIN  60   // Minimum time to trigger a short button press

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
PanelManager::PanelManager()
    : displayAvailable(false), topicIndex(0), statusTopic("Status"), infoTopic("Info"), configTopic("Config"), commandTopic("Command"),
      currentTopic(nullptr)
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

        topicIndex = 0;

        commandMutex      = portMUX_INITIALIZER_UNLOCKED;
        buttonMutex       = portMUX_INITIALIZER_UNLOCKED;
        commandEventGroup = xEventGroupCreate();
        xTaskCreate(CommandProcessingTask, "DioTask", 16384, (void *)this, 5, &commandTaskHandle);
    }

    depthPage.SetData(&PageHandler::navData.dpt_m, std::string("m"), true, &PageHandler::navData.stp_degc, std::string("c"), true);
    speedPage.SetData(&PageHandler::navData.spd_kt, std::string("kt"), true, &PageHandler::navData.vcc_v, std::string("v"), true);
    trueWindPage.SetData(&PageHandler::navData.tws_kt, std::string("kt"), true, &PageHandler::navData.twa_deg, std::string("o"), false);

    statusTopic.AddPage(&clockPage, "Data: Time, Date");
    statusTopic.AddPage(&depthPage, "Data: DPT, STP");
    statusTopic.AddPage(&speedPage, "Data: SPD, VCC");
    statusTopic.AddPage(&trueWindPage, "Data: TWS, TWA");
    topicList.push_back(&statusTopic);

    infoTopic.AddPage(&networkPage, "Info: RF quality");
    infoTopic.AddPage(&infoPagePower, "Info: Battery");
    infoTopic.AddPage(&infoPageMicronet, "Info: Micronet");
    infoTopic.AddPage(&infoPageSensors, "Info: Sensors");
    infoTopic.AddPage(&infoPageCompass, "Info: Compass");
    topicList.push_back(&infoTopic);

    configTopic.AddPage(&configPage1, "Config: General");
    configTopic.AddPage(&configPage2, "Config: Links");
    topicList.push_back(&configTopic);

    commandTopic.AddPage(&commandPage, "Commands");
    topicList.push_back(&commandTopic);

    currentTopic = &statusTopic;
    topicIndex   = 0;

    // Register button's ISR
    pinMode(BUTTON_PIN, INPUT);
    objectPtr = this;
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), StaticButtonIsr, CHANGE);

    // Register Power button callback
    gPower.RegisterButtonCallback(PowerButtonCallback);

    DrawPage();

    return displayAvailable;
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
void PanelManager::NextTopic()
{
    portENTER_CRITICAL(&commandMutex);
    this->topicIndex = (topicIndex + 1) < topicList.size() ? topicIndex + 1 : 0;
    portEXIT_CRITICAL(&commandMutex);
}

/*
  Request change of the current page (callable from ISR)
*/
void PanelManager::NextTopicISR()
{
    portENTER_CRITICAL_ISR(&commandMutex);
    this->topicIndex = (topicIndex + 1) < topicList.size() ? topicIndex + 1 : 0;
    portEXIT_CRITICAL_ISR(&commandMutex);
}

/*
  Gives PanelManager the latest version of navigation data for pages which needs it
  @param navData Pointer to the latest navigation dataset
*/
void PanelManager::SetNavigationData(NavigationData &navData)
{
    // FIXME : Work on a copy of data instead of a direct pointer
    portENTER_CRITICAL(&commandMutex);
    PageHandler::SetNavData(navData);
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
        // First, we read the actual button IO status
        int buttonStatus = !digitalRead(BUTTON_PIN);
        // Then, we read the button status as seen by the ISR
        localLastPress     = lastPress;
        localLastRelease   = lastRelease;
        localButtonPressed = buttonPressed;
        // If different, it means ISR missed a status change, likely because of a rebound
        if (localButtonPressed != buttonStatus)
        {
            // So we update the status of the button and update the press/release times
            localButtonPressed = buttonStatus;
            if (buttonStatus == true)
            {
                localLastPress = now;
            }
            else
            {
                localLastRelease = now;
            }
        }
        portEXIT_CRITICAL(&buttonMutex);

        // Do we have a long press on the button ?
        if ((localButtonPressed == true) && ((now - localLastPress) > BUTTON_LONG_PRESS_DELAY) && !longPress)
        {
            // Button is pressed for more than the long press time : handle long press processing
            longPress = true;
            // In case of long press, we consider the button released
            portENTER_CRITICAL(&buttonMutex);
            buttonPressed = false;
            portEXIT_CRITICAL(&buttonMutex);
            // Request action to the currently displayed page
            switch (currentTopic->OnButtonPressed(BUTTON_ID_1, true))
            {
            case PAGE_ACTION_EXIT_TOPIC:
            case PAGE_ACTION_EXIT_PAGE:
                // Go to next page
                NextTopic();
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
            else if (((localLastRelease - lastPageUpdate) > BUTTON_SHORT_PRESS_MIN) && ((localLastRelease - localLastPress) > BUTTON_SHORT_PRESS_MIN))
            {
                // Yes : we have a short press
                // Request action to the currently displayed page
                switch (currentTopic->OnButtonPressed(BUTTON_ID_1, false))
                {
                case PAGE_ACTION_EXIT_TOPIC:
                case PAGE_ACTION_EXIT_PAGE:
                    // Go to next page
                    NextTopic();
                    commandFlags |= COMMAND_EVENT_NEW_PAGE;
                    break;
                case PAGE_ACTION_REFRESH:
                    // Refresh current page
                    commandFlags |= COMMAND_EVENT_REFRESH;
                    break;
                }
            }
        }

        // Has button been released ?
        if (commandFlags & (COMMAND_EVENT_POWER_BUTTON_SHORT | COMMAND_EVENT_POWER_BUTTON_LONG))
        {
            // Yes : we have a short press
            // Request action to the currently displayed page
            switch (currentTopic->OnButtonPressed(BUTTON_ID_0, commandFlags & COMMAND_EVENT_POWER_BUTTON_LONG))
            {
            case PAGE_ACTION_EXIT_TOPIC:
                // Go to next page
                NextTopic();
                commandFlags |= COMMAND_EVENT_NEW_PAGE;
                break;
            case PAGE_ACTION_REFRESH:
                // Refresh current page
                commandFlags |= COMMAND_EVENT_REFRESH;
                break;
            }
        }

        // Process page change/refresh if requested
        if ((commandFlags & COMMAND_EVENT_REFRESH) || (commandFlags & COMMAND_EVENT_NEW_PAGE) || ((now - lastPageUpdate) > DISPLAY_UPDATE_PERIOD))
        {
            lastPageUpdate = now;
            portENTER_CRITICAL(&commandMutex);
            currentTopic = topicList.at(topicIndex);
            portEXIT_CRITICAL(&commandMutex);

            currentTopic->Draw(commandFlags & (COMMAND_EVENT_NEW_PAGE | COMMAND_EVENT_REFRESH));
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
    int        buttonStatus   = !digitalRead(BUTTON_PIN);
    uint32_t   now            = millis();
    BaseType_t scheduleChange = pdFALSE;

    if (buttonStatus)
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
  Function called each time power button status changed
  @param longPress True if power button has been pressed for a long time (i.e. >=1s)
*/
void PanelManager::PowerButtonCallback(bool longPress)
{
    if (longPress)
    {
        xEventGroupSetBits(objectPtr->commandEventGroup, COMMAND_EVENT_POWER_BUTTON_LONG);
    }
    else
    {
        xEventGroupSetBits(objectPtr->commandEventGroup, COMMAND_EVENT_POWER_BUTTON_SHORT);
    }
}

/*
  Enable/disable panel low power mode
*/
void PanelManager::LowPower(bool enable)
{
    display.dim(enable);
}
