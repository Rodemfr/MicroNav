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

/***************************************************************************/
/*                              Functions                                  */
/***************************************************************************/

/*
    Class constructor
*/
TopicHandler::TopicHandler(std::string topicName)
{
    this->topicName = topicName;
    pageIndex = 0;
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
void TopicHandler::Draw(bool force, bool flushDisplay)
{
    char lineStr[22];
    int  listSize = pageList.size();
    int  i;

    if (listSize > pageIndex)
    {
        pageList.at(pageIndex)->Draw(force, false);
    }

    display->setTextSize(1);
    display->setFont(nullptr);
    display->setTextColor(SSD1306_WHITE);
    snprintf(lineStr, sizeof(lineStr), "%s %d/%d", topicName.c_str(), pageIndex + 1, listSize);
    PrintLeft(56, lineStr);
    if (flushDisplay)
    {
        display->display();
    }
}

void TopicHandler::AddPage(PageHandler *pageHandler)
{
    pageList.push_back(pageHandler);
}

PageAction_t TopicHandler::OnButtonPressed(ButtonId_t buttonId, bool longPress)
{
    PageAction_t action = PAGE_ACTION_EXIT_TOPIC;

    if (pageList.size() >= pageIndex)
    {
        action = pageList.at(pageIndex)->OnButtonPressed(buttonId, longPress);
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
