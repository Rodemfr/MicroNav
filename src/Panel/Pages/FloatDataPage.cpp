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

#include "FloatDataPage.h"
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

FloatDataPage::FloatDataPage() : prevValue1(0), prevValue2(0), prevValue1Valid(false), prevValue2Valid(false)
{
    this->value1 = nullptr;
    this->value2 = nullptr;
    fracDigit1   = false;
    fracDigit2   = false;
}

FloatDataPage::~FloatDataPage()
{
}

void FloatDataPage::SetData(FloatValue_t *value, std::string unit, bool fracDigit)
{
    this->value1     = value;
    this->unit1      = unit;
    this->fracDigit1 = fracDigit1;
    this->value2     = nullptr;
    this->unit2      = std::string("");
    this->fracDigit2 = fracDigit2;
}

void FloatDataPage::SetData(FloatValue_t *value1, std::string unit1, bool fracDigit1, FloatValue_t *value2, std::string unit2, bool fracDigit2)
{
    this->value1 = value1;
    this->unit1  = unit1;
    this->fracDigit1 = fracDigit1;
    this->value2 = value2;
    this->unit2  = unit2;
    this->fracDigit2 = fracDigit2;
}

bool FloatDataPage::Draw(bool force, bool flushDisplay)
{
    char     lineStr1V[12];
    char     lineStr1F[12];
    char     lineStr2V[12];
    char     lineStr2F[12];
    int16_t  xS1, yS1, xS2, yS2, xS3, yS3;
    uint16_t wS1, hS1, wS2, hS2, wS3, hS3;
    int16_t unitVShift = 0;
    bool     updateDisplay = false;

    int32_t intValue1 = 0;
    int32_t intValue2 = 0;

    if (value1 != nullptr)
    {
        intValue1 = value1->value * 10;
    }
    if (value1 != nullptr)
    {
        intValue2 = value2->value * 10;
    }

    if ((value1->valid != prevValue1Valid) || (intValue1 != prevValue1) || (intValue2 != prevValue2) || (value1->valid != prevValue2Valid))
    {
        prevValue1Valid = navData.dpt_m.valid;
        prevValue1      = intValue1;
        prevValue2Valid = navData.vcc_v.valid;
        prevValue2      = intValue2;

        updateDisplay = true;
    }

    if (updateDisplay || force)
    {
        display->clearDisplay();
        display->setTextColor(SSD1306_WHITE);
        display->setTextSize(1);

        if (value1 != nullptr)
        {
            display->setFont(&FreeSansBold18pt);
            if (value1->valid)
            {
                snprintf(lineStr1V, sizeof(lineStr1V), "%d", intValue1 / 10);
            }
            else
            {
                snprintf(lineStr1V, sizeof(lineStr1V), "--");
            }
            display->getTextBounds(String(lineStr1V), 0, 0, &xS1, &yS1, &wS1, &hS1);

            if (fracDigit1)
            {
                display->setFont(&FreeSansBold12pt);
                if (value1->valid)
                {
                    snprintf(lineStr1F, sizeof(lineStr1F), ".%d", intValue1 % 10);
                }
                else
                {
                    snprintf(lineStr1F, sizeof(lineStr1F), ".-");
                }
                display->getTextBounds(String(lineStr1F), 0, 0, &xS2, &yS2, &wS2, &hS2);
            }
            else
            {
                wS2 = 0;
            }

            if (unit1.compare("o") == 0)
            {
                display->setFont(&FreeSansBold9pt);
                unitVShift = -8;
            }
            else
            {
                display->setFont(&FreeSansBold12pt);
                unitVShift = 0;
            }
            display->getTextBounds(String(unit1.c_str()), 0, 0, &xS3, &yS3, &wS3, &hS3);
            int16_t xShift = (SCREEN_WIDTH - (wS1 + wS2 + wS3 + 8)) / 2;

            display->setFont(&FreeSansBold18pt);
            display->setCursor(xShift, 24);
            display->print(lineStr1V);

            display->setFont(&FreeSansBold12pt);
            if (fracDigit1)
            {
                display->setCursor(xShift + wS1 + 3, 24);
                display->print(lineStr1F);
            }

            if (unit1.compare("o") == 0)
            {
                display->setFont(&FreeSansBold9pt);
            }
            display->setCursor(xShift + wS1 + 3 + wS2 + 5, 24 + unitVShift);
            display->print(unit1.c_str());
        }

        if (value2 != nullptr)
        {
            display->setFont(&FreeSansBold18pt);
            if (value2->valid)
            {
                snprintf(lineStr2V, sizeof(lineStr2V), "%d", intValue2 / 10);
            }
            else
            {
                snprintf(lineStr2V, sizeof(lineStr2V), "--");
            }
            display->getTextBounds(String(lineStr2V), 0, 0, &xS1, &yS1, &wS1, &hS1);

            if (fracDigit2)
            {
                display->setFont(&FreeSansBold12pt);
                if (value2->valid)
                {
                    snprintf(lineStr2F, sizeof(lineStr2F), ".%d", intValue2 % 10);
                }
                else
                {
                    snprintf(lineStr2F, sizeof(lineStr2F), ".-");
                }
                display->getTextBounds(String(lineStr2F), 0, 0, &xS2, &yS2, &wS2, &hS2);
            }
            else
            {
                wS2 = 0;
            }

            if (unit2.compare("o") == 0)
            {
                display->setFont(&FreeSansBold9pt);
                unitVShift = -8;
            }
            else
            {
                display->setFont(&FreeSansBold12pt);
                unitVShift = 0;
            }
            display->getTextBounds(String(unit2.c_str()), 0, 0, &xS3, &yS3, &wS3, &hS3);
            int16_t xShift = (SCREEN_WIDTH - (wS1 + wS2 + wS3 + 8)) / 2;

            display->setFont(&FreeSansBold18pt);
            display->setCursor(xShift, 24 + 28);
            display->print(lineStr2V);

            display->setFont(&FreeSansBold12pt);
            if (fracDigit2)
            {
                display->setCursor(xShift + wS1 + 3, 24 + 28);
                display->print(lineStr2F);
            }

            if (unit2.compare("o") == 0)
            {
                display->setFont(&FreeSansBold9pt);
            }
            display->setCursor(xShift + wS1 + 3 + wS2 + 5, 24 + 28 + unitVShift);
            display->print(unit2.c_str());
        }

        if (flushDisplay)
        {
            display->display();
        }
    }

    return (updateDisplay || force);
}
