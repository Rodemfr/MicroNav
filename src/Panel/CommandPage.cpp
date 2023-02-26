/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Handler of the Command page                                   *
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

#include "CommandPage.h"
#include "Globals.h"
#include "MicronetDevice.h"
#include "PanelResources.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

// @brief Configuration items int the menu
typedef enum
{
    COMMAND_POSITION_SHUTDOWN = 0,
    COMMAND_POSITION_ATTACH,
    COMMAND_POSITION_CALCOMPASS,
    COMMAND_POSITION_EXIT
} CmdPos_t;

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

CommandPage::CommandPage() : editMode(false), editPosition(0), subPage(nullptr)
{
}

CommandPage::~CommandPage()
{
}

/*
  Draw the page on display
  @param force Force redraw, even if the content did not change
*/
void CommandPage::Draw(bool force)
{
    char     lineStr[22];
    int16_t  xStr, yStr;
    uint16_t wStr, hStr;

    if (subPage != nullptr)
    {
        subPage->Draw(force);
    }
    else if (display != nullptr)
    {
        display->clearDisplay();

        // Config items
        display->setTextSize(1);
        display->setFont(nullptr);

        // TODO : Factorize draw of selection background
        if ((editPosition == COMMAND_POSITION_SHUTDOWN) && (editMode))
        {
            display->fillRect(0, 0 * 8, SCREEN_WIDTH, 8, SSD1306_WHITE);
            display->setTextColor(SSD1306_BLACK);
        }
        else
        {
            display->setTextColor(SSD1306_WHITE);
        }
        PrintCentered(0 * 8, "Shutdown MicroNav");

        if ((editPosition == COMMAND_POSITION_ATTACH) && (editMode))
        {
            display->fillRect(0, 1 * 8, SCREEN_WIDTH, 8, SSD1306_WHITE);
            display->setTextColor(SSD1306_BLACK);
        }
        else
        {
            display->setTextColor(SSD1306_WHITE);
        }
        PrintCentered(1 * 8, "Attach to network");

        if ((editPosition == COMMAND_POSITION_CALCOMPASS) && (editMode))
        {
            display->fillRect(0, 2 * 8, SCREEN_WIDTH, 8, SSD1306_WHITE);
            display->setTextColor(SSD1306_BLACK);
        }
        else
        {
            display->setTextColor(SSD1306_WHITE);
        }
        PrintCentered(2 * 8, "Calibrate compass");

        if (editMode)
        {
            if (editPosition == COMMAND_POSITION_EXIT)
            {
                display->fillRect(0, 64 - 8, SCREEN_WIDTH, 8, SSD1306_WHITE);
                display->setTextColor(SSD1306_BLACK);
            }
            else
            {
                display->setTextColor(SSD1306_WHITE);
            }
            PrintCentered(64 - 8, "Exit");
        }
        else
        {
            display->setTextColor(SSD1306_WHITE);
            PrintCentered(64 - 8, "Commands");
        }

        display->display();
    }
}

// @brief Function called by PanelManager when the button is pressed
// @param longPress true if a long press was detected
// @return Action to be executed by PanelManager
PageAction_t CommandPage::OnButtonPressed(bool longPress)
{
    // Check if we are currently displaying a sub page
    if (subPage != nullptr)
    {
        // Yes : does the page request exit ?
        PageAction_t subAction = subPage->OnButtonPressed(longPress);
        if (subAction == PAGE_ACTION_EXIT)
        {
            // Yes : leave sub page mode and request a refresh of the page
            subPage = nullptr;
            return PAGE_ACTION_REFRESH;
        }
        // No : return action from the sub page
        return subAction;
    }

    PageAction_t action = PAGE_ACTION_EXIT;

    if (editMode)
    {
        // In edit mode, the button is used to cycle through the configuration items
        if (longPress)
        {
            if (editPosition == COMMAND_POSITION_EXIT)
            {
                // Long press on "Exit"
                editMode = false;
            }
            else
            {
                // Long press on a command
                switch (editPosition)
                {
                case 0:
                    gPower.Shutdown();
                    break;
                case 1:
                    subPage = &attachPage;
                    break;
                }
            }
            action = PAGE_ACTION_REFRESH;
        }
        else
        {
            // Short press : cycle through configuration items
            editPosition = (editPosition + 1) % (COMMAND_POSITION_EXIT + 1);
            action       = PAGE_ACTION_REFRESH;
        }
    }
    else
    {
        if (longPress)
        {
            // Long press while not in edit mode : enter edit mode
            editMode     = true;
            editPosition = 0;
            action       = PAGE_ACTION_REFRESH;
        }
        else
        {
            // Short press while not in edit mode : cycle to next page
            action = PAGE_ACTION_EXIT;
        }
    }

    return action;
}
