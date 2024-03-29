/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Handler of the Logo page                                      *
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

#include "LogoPage.h"
#include "Globals.h"
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

/***************************************************************************/
/*                              Functions                                  */
/***************************************************************************/

LogoPage::LogoPage() : swMajorVersion(0), swMinorVersion(0), swPatchVersion(0)
{
}

LogoPage::~LogoPage()
{
}

void LogoPage::SetSwversion(uint8_t swMajorVersion, uint8_t swMinorVersion, uint32_t swPatchVersion)
{
    this->swMajorVersion = swMajorVersion;
    this->swMinorVersion = swMinorVersion;
    this->swPatchVersion = swPatchVersion;
}

bool LogoPage::Draw(bool force, bool flushDisplay)
{
    char     versionStr[10];
    char     networkIdStr[9];
    int16_t  xVersion, yVersion;
    uint16_t wVersion, hVersion;
    bool drawed = false;

    if (display != nullptr)
    {
        drawed = true;
        display->clearDisplay();
        display->drawBitmap(0, 0, LOGO_BITMAP, LOGO_WIDTH, LOGO_HEIGHT, 1);

        display->setTextColor(SSD1306_WHITE);
        display->setTextSize(1);
        display->setFont(nullptr);
        snprintf(versionStr, sizeof(versionStr), "v%d.%d.%d", swMajorVersion, swMinorVersion, swPatchVersion);
        display->getTextBounds(String(versionStr), 0, 0, &xVersion, &yVersion, &wVersion, &hVersion);
        display->setCursor(SCREEN_WIDTH - wVersion, LOGO_HEIGHT - yVersion + 2);
        display->print(versionStr);
        if (flushDisplay)
        {
            display->display();
        }
    }

    return drawed;
}