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

#ifndef LOGOPAGE_H_
#define LOGOPAGE_H_

 /***************************************************************************/
 /*                              Includes                                   */
 /***************************************************************************/

#include "PageHandler.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

/***************************************************************************/
/*                                Types                                    */
/***************************************************************************/

/***************************************************************************/
/*                               Classes                                   */
/***************************************************************************/

class LogoPage : public PageHandler
{
public:
    LogoPage();
    virtual ~LogoPage();

    void SetDisplay(Adafruit_SSD1306 *display);
    void Draw();
    void UpdateStatus();

private:
    Adafruit_SSD1306 *display;

};

#endif