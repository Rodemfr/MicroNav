/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Ressources for Panel and Pages                                *
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

#ifndef PANELRESOURCES_H_
#define PANELRESOURCES_H_

/***************************************************************************/
/*                              Includes                                   */
/***************************************************************************/

#include <Adafruit_GFX.h>
#include <Arduino.h>

#include "bitmaps/logo.h"
#include "bitmaps/t000.h"
#include "bitmaps/t060.h"
#include "bitmaps/t075.h"
#include "bitmaps/t110.h"
#include "bitmaps/t111.h"
#include "bitmaps/t112.h"
#include "bitmaps/t113.h"
#include "bitmaps/t120.h"
#include "bitmaps/t121.h"
#include "bitmaps/t210.h"
#include "bitmaps/t215.h"

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

/***************************************************************************/
/*                              Prototypes                                 */
/***************************************************************************/

extern GFXfont FreeSansBold9pt;
extern GFXfont FreeSansBold12pt;
extern GFXfont FreeSansBold24pt;

#endif
