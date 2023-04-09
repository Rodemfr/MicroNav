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

/***************************************************************************/
/*                              Includes                                   */
/***************************************************************************/

#include <Adafruit_GFX.h>
#include <Arduino.h>

#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

GFXfont FreeSansBold9pt = {(uint8_t *)FreeSansBold9pt7bBitmaps, (GFXglyph *)FreeSansBold9pt7bGlyphs, 0x20, 0x7E, 22};
GFXfont FreeSansBold12pt = {(uint8_t *)FreeSansBold12pt7bBitmaps, (GFXglyph *)FreeSansBold12pt7bGlyphs, 0x20, 0x7E, 29};
GFXfont FreeSansBold18pt = {(uint8_t *)FreeSansBold18pt7bBitmaps, (GFXglyph *)FreeSansBold18pt7bGlyphs, 0x20, 0x7E, 42};

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