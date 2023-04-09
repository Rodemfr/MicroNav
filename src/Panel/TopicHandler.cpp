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

#include "TopicHandler.h"
#include "PanelResources.h"
#include "Globals.h"

#include <Arduino.h>
#include <cmath>

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

/***************************************************************************/
/*                              Functions                                  */
/***************************************************************************/

/*
    Class constructor
*/
TopicHandler::TopicHandler(std::string topicName)
{
    this->topicName = topicName;
    pageIndex       = 0;
}

/*
    Class destructor
*/
TopicHandler::~TopicHandler()
{
}

/*
    Draw the page on display panel
    @param force Force complete redraw of the page, even if there are no changes.
*/
bool TopicHandler::Draw(bool force, bool flushDisplay)
{
    int  listSize = pageList.size();
    int  i;
    bool drawed = false;

    if (listSize > pageIndex)
    {
        drawed = pageList.at(pageIndex).handler->Draw(force, false);
    }

    if (drawed)
    {
        display->setTextSize(1);
        display->setFont(nullptr);
        display->setTextColor(SSD1306_WHITE);
        PrintLeft(56, pageList.at(pageIndex).name.c_str());
        DrawBatteryStatus(SCREEN_WIDTH - 22, 56, lroundf(gPower.GetStatus().batteryLevel_per));
        if (flushDisplay)
        {
            display->display();
        }
    }

    return drawed;
}

void TopicHandler::AddPage(PageHandler *pageHandler, std::string name)
{
    PageRef_t pageRef;

    pageRef.name    = name;
    pageRef.handler = pageHandler;

    pageList.push_back(pageRef);
}

PageAction_t TopicHandler::OnButtonPressed(ButtonId_t buttonId, bool longPress)
{
    PageAction_t action = PAGE_ACTION_EXIT_TOPIC;

    if (pageList.size() >= pageIndex)
    {
        action = pageList.at(pageIndex).handler->OnButtonPressed(buttonId, longPress);
    }

    if (action == PAGE_ACTION_EXIT_PAGE)
    {
        pageIndex = (pageIndex + 1);
        if (pageIndex >= pageList.size())
        {
            pageIndex = 0;
        }
        action = PAGE_ACTION_REFRESH;
    }

    return action;
}

void TopicHandler::DrawBatteryStatus(uint32_t x, uint32_t y, uint32_t level)
{
    uint32_t width;

    if (level > 100)
        level = 100;

    width = 16 * level / 100;

    display->drawRect(x, y, 18, 8, SSD1306_WHITE);
    display->fillRect(x + 18, y + 2, 2, 4, SSD1306_WHITE);
    if (width > 0)
    {
        display->fillRect(x + 1, y + 1, width, 6, SSD1306_WHITE);
    }
}