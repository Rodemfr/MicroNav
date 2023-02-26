/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Handler of the Clock page                                     *
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

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

const int gMonthLength[13] = {0, 31, 30, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

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
    : prevNavDataValid(false), prevHour(0), prevMinute(0), prevDay(0), prevMonth(0), prevYear(0), prevTimeValid(false), prevDateValid(false)
{
}

ClockPage::~ClockPage()
{
}

void ClockPage::Draw(bool force)
{
    char     timeStr[] = "--:--";
    char     dateStr[] = "--/--/----";
    int16_t  xTime, yTime, xDate, yDate;
    uint16_t wTime, hTime, wDate, hDate;
    bool     updateDisplay = false;
    int      minute, hour, day, month, year;
    int      dayShift = 0;

    if (navData != nullptr)
    {
        if (navData->time.valid != prevTimeValid)
        {
            updateDisplay = true;
            prevTimeValid = navData->time.valid;
        }

        if (navData->time.valid)
        {
            minute = navData->time.minute;
            hour   = navData->time.hour;
            hour += navData->timeZone_h;
            if (hour < 0)
            {
                hour += 24;
                dayShift = -1;
            }
            else if (hour > 23)
            {
                hour -= 24;
                dayShift = 1;
            }

            timeStr[0] = (hour / 10) + '0';
            timeStr[1] = (hour % 10) + '0';
            timeStr[3] = (minute / 10) + '0';
            timeStr[4] = (minute % 10) + '0';

            if ((hour != prevHour) || (minute != prevMinute))
            {
                updateDisplay = true;
                prevHour      = hour;
                prevMinute    = minute;
            }
        }
        else
        {
            timeStr[0] = '-';
            timeStr[1] = '-';
            timeStr[3] = '-';
            timeStr[4] = '-';
        }

        if (navData->date.valid != prevDateValid)
        {
            updateDisplay = true;
            prevDateValid = navData->date.valid;
        }

        if (navData->date.valid)
        {
            day   = navData->date.day + dayShift;
            month = navData->date.month;
            year  = navData->date.year;

            int monthLength = gMonthLength[month];
            if ((month == 2) && ((year % 4) == 0))
            {
                monthLength += 1;
            }

            if (day > monthLength)
            {
                day = 1;
                month += 1;
                if (month > 12)
                {
                    month = 1;
                    year += 1;
                }
            }

            dateStr[0] = (day / 10) + '0';
            dateStr[1] = (day % 10) + '0';
            dateStr[3] = (month / 10) + '0';
            dateStr[4] = (month % 10) + '0';
            dateStr[6] = '2';
            dateStr[7] = '0';
            dateStr[8] = ((year / 10) % 10) + '0';
            dateStr[9] = (year % 10) + '0';

            if ((navData->date.day != prevDay) || (navData->date.month != prevMonth) || (navData->date.year != prevYear))
            {
                updateDisplay = true;
                prevDay       = day;
                prevMonth     = month;
                prevYear      = year;
            }
        }
        else
        {
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

    if (prevNavDataValid != (navData != nullptr))
    {
        prevNavDataValid = (navData != nullptr);
        updateDisplay    = true;
    };

    if (updateDisplay || force)
    {
        display->clearDisplay();
        display->setTextColor(SSD1306_WHITE);
        display->setTextSize(1);
        display->setFont(&FreeSansBold24pt);
        display->getTextBounds(String(timeStr), 0, 0, &xTime, &yTime, &wTime, &hTime);
        display->setFont(&FreeSansBold9pt);
        display->getTextBounds(dateStr, 0, 0, &xDate, &yDate, &wDate, &hDate);

        display->setFont(&FreeSansBold24pt);
        display->setCursor((SCREEN_WIDTH - wTime) / 2, -yTime + (SCREEN_HEIGHT + yTime + yDate - 6) / 2);
        display->println(timeStr);

        display->setFont(&FreeSansBold9pt);
        display->setCursor((SCREEN_WIDTH - wDate) / 2, -yTime - yDate + 6 + (SCREEN_HEIGHT + yTime + yDate - 6) / 2);
        display->println(dateStr);
        display->display();
    }
}

void ClockPage::SetNavData(NavigationData *navData)
{
    this->navData = navData;
}
