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

#ifndef TOPICHANDLER_H_
#define TOPICHANDLER_H_

/***************************************************************************/
/*                              Includes                                   */
/***************************************************************************/

#include "MicronetDevice.h"
#include "PageHandler.h"

#include <vector>
#include <string>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

/***************************************************************************/
/*                                Types                                    */
/***************************************************************************/

typedef struct
{
    PageHandler *handler;
    std::string name;
} PageRef_t;

/***************************************************************************/
/*                               Classes                                   */
/***************************************************************************/

class TopicHandler : public PageHandler
{
    public:
        TopicHandler(std::string topicName);
        virtual ~TopicHandler();
        void AddPage(PageHandler *pageHandler, std::string pageName);
        virtual bool Draw(bool force, bool flushDisplay = true);
        virtual PageAction_t OnButtonPressed(ButtonId_t buttonId, bool longPress);

    protected:
        std::string topicName;
        uint32_t pageIndex;
        std::vector<PageRef_t> pageList;

        void DrawBatteryStatus(uint32_t x, uint32_t y, uint32_t level);
};

#endif
