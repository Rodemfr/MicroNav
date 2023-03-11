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

#ifndef PAGEHANDLER_H_
#define PAGEHANDLER_H_

/***************************************************************************/
/*                              Includes                                   */
/***************************************************************************/

#include "MicronetDevice.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

/***************************************************************************/
/*                                Types                                    */
/***************************************************************************/

typedef enum
{
    PAGE_ACTION_NONE = 0,
    PAGE_ACTION_EXIT,
    PAGE_ACTION_REFRESH
} PageAction_t;

typedef enum
{
  BUTTON_ID_0 = 0,
  BUTTON_ID_1
} ButtonId_t;

/***************************************************************************/
/*                               Classes                                   */
/***************************************************************************/

class PageHandler
{
  public:
    PageHandler();
    virtual ~PageHandler() = 0;

    static void          SetDisplay(Adafruit_SSD1306 *display);
    virtual void         Draw(bool force) = 0;
    virtual PageAction_t OnButtonPressed(ButtonId_t buttonId, bool longPress);
    static void          SetNetworkStatus(DeviceInfo_t &deviceInfo);

  protected:
    static Adafruit_SSD1306 *display;
    static DeviceInfo_t      deviceInfo;

    void PrintCentered(int32_t yPos, String const &text);
    void PrintCentered(int32_t xPos, int32_t yPos, String const &text);
    void PrintLeft(int32_t yPos, String const &text);
    void PrintRight(int32_t yPos, String const &text);
};

#endif
