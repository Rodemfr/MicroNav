/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Clock page handler                                            *
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

#include "ClockPage.h"
#include "PanelResources.h"

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

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

ClockPage::ClockPage()
{
}

ClockPage::~ClockPage()
{
}

void ClockPage::Draw()
{
    String time = "22:17";
    String date = "05/12/2022";
    int16_t xTime, yTime, xDate, yDate;
    uint16_t wTime, hTime, wDate, hDate;

    display->clearDisplay();
    display->setTextColor(SSD1306_WHITE);
    display->setTextSize(1);
    display->setFont(&FreeSansBold24pt);
    display->getTextBounds(time, 0, 0, &xTime, &yTime, &wTime, &hTime);
    display->setFont(&FreeSansBold12pt);
    display->getTextBounds(date, 0, 0, &xDate, &yDate, &wDate, &hDate);

    display->setFont(&FreeSansBold24pt);
    display->setCursor((SCREEN_WIDTH - wTime) / 2, -yTime + (SCREEN_HEIGHT + yTime + yDate - 6) / 2);
    display->println(time);

    display->setFont(&FreeSansBold12pt);
    display->setCursor((SCREEN_WIDTH - wDate) / 2, -yTime - yDate + 6 + (SCREEN_HEIGHT + yTime + yDate - 6) / 2);
    display->println(date);
    display->display();
}

void ClockPage::UpdateStatus()
{
}
