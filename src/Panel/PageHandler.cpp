/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Page Handler abstract class                                   *
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

#include "PageHandler.h"
#include "MicronetDevice.h"
#include "PanelResources.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

/***************************************************************************/
/*                             Local types                                 */
/***************************************************************************/

/***************************************************************************/
/*                           Local prototypes                              */
/***************************************************************************/

/***************************************************************************/
/*                           Static & Globals                              */
/***************************************************************************/

Adafruit_SSD1306 *PageHandler::display;
DeviceInfo_t      PageHandler::deviceInfo;

/***************************************************************************/
/*                              Functions                                  */
/***************************************************************************/

PageHandler::PageHandler()
{
}

PageHandler::~PageHandler()
{
}

void PageHandler::SetDisplay(Adafruit_SSD1306 *display)
{
    PageHandler::display = display;
}

PageAction_t PageHandler::OnButtonPressed(bool longPress)
{
    return (longPress ? PAGE_ACTION_NONE : PAGE_ACTION_EXIT);
}

void PageHandler::PrintCentered(int32_t yPos, String const &text)
{
    PrintCentered(SCREEN_WIDTH / 2, yPos, text);
}

void PageHandler::PrintCentered(int32_t xPos, int32_t yPos, String const &text)
{
    int16_t  xStr, yStr;
    uint16_t wStr, hStr;

    display->getTextBounds(text, 0, 0, &xStr, &yStr, &wStr, &hStr);
    display->setCursor(xPos - (wStr / 2), yPos);
    display->println(text);
}

void PageHandler::PrintLeft(int32_t yPos, String const &text)
{
    display->setCursor(0, yPos);
    display->println(text);
}

void PageHandler::PrintRight(int32_t yPos, String const &text)
{
    int16_t  xStr, yStr;
    uint16_t wStr, hStr;

    display->getTextBounds(text, 0, 0, &xStr, &yStr, &wStr, &hStr);
    display->setCursor(SCREEN_WIDTH - wStr, yPos);
    display->println(text);
}

/*
  Set the latest network status.
  @param deviceInfo Structure
*/
void PageHandler::SetNetworkStatus(DeviceInfo_t &deviceInfo)
{
    // Copy it in a static local variable so that every child of PageHandler class will be able to access it
    PageHandler::deviceInfo = deviceInfo;
}
