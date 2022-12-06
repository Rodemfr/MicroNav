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

ClockPage::ClockPage() : prevHour(0), prevMinute(0), prevDay(0), prevMonth(0), prevYear(0), prevTimeValid(0), prevDateValid(0)
{
}

ClockPage::~ClockPage()
{
}

void ClockPage::Draw()
{
    char timeStr[] = "--:--";
    char dateStr[] = "--/--/----";
    int16_t xTime, yTime, xDate, yDate;
    uint16_t wTime, hTime, wDate, hDate;
    bool updateDisplay = false;

    if (navData != nullptr)
    {
        if ((navData->time.valid != prevTimeValid) || (navData->time.hour != prevHour) || (navData->time.minute != prevMinute))
        {
            updateDisplay = true;
            prevTimeValid = navData->time.valid;
            prevHour = navData->time.hour;
            prevMinute = navData->time.minute;
        }

        if (navData->time.valid)
        {
            timeStr[0] = (navData->time.hour / 10) + '0';
            timeStr[1] = (navData->time.hour % 10) + '0';
            timeStr[3] = (navData->time.minute / 10) + '0';
            timeStr[4] = (navData->time.minute % 10) + '0';
        }
        else {
            timeStr[0] = '-';
            timeStr[1] = '-';
            timeStr[3] = '-';
            timeStr[4] = '-';
        }

        if ((navData->date.valid != prevDateValid) || (navData->date.day != prevDay) || (navData->date.month != prevMonth) || (navData->date.year != prevYear))
        {
            updateDisplay = true;
            prevDateValid = navData->date.valid;
            prevDay = navData->date.day;
            prevMonth = navData->date.month;
            prevYear = navData->date.year;
        }

        if (navData->date.valid)
        {
            dateStr[0] = (navData->date.day / 10) + '0';
            dateStr[1] = (navData->date.day % 10) + '0';
            dateStr[3] = (navData->date.month / 10) + '0';
            dateStr[4] = (navData->date.month % 10) + '0';
            dateStr[6] = '2';
            dateStr[7] = '0';
            dateStr[8] = ((navData->date.year / 10) % 10) + '0';
            dateStr[9] = (navData->date.year % 10) + '0';
        }
        else {
            dateStr[0] = '-';
            dateStr[1] = '-';
            dateStr[3] = '-';
            dateStr[4] = '-';
            dateStr[6] = '-';
            dateStr[6] = '-';
            dateStr[7] = '-';
            dateStr[8] = '-';
        }
    }

    if (updateDisplay)
    {
        display->clearDisplay();
        display->setTextColor(SSD1306_WHITE);
        display->setTextSize(1);
        display->setFont(&FreeSansBold24pt);
        display->getTextBounds(String(timeStr), 0, 0, &xTime, &yTime, &wTime, &hTime);
        display->setFont(&FreeSansBold12pt);
        display->getTextBounds(dateStr, 0, 0, &xDate, &yDate, &wDate, &hDate);

        display->setFont(&FreeSansBold24pt);
        display->setCursor((SCREEN_WIDTH - wTime) / 2, -yTime + (SCREEN_HEIGHT + yTime + yDate - 6) / 2);
        display->println(timeStr);

        display->setFont(&FreeSansBold12pt);
        display->setCursor((SCREEN_WIDTH - wDate) / 2, -yTime - yDate + 6 + (SCREEN_HEIGHT + yTime + yDate - 6) / 2);
        display->println(dateStr);
        display->display();
    }
}
