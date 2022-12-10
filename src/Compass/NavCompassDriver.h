/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Compass driver abstract class                                 *
 * Author:   Ronan Demoment                                                *
 *                                                                         *
 ***************************************************************************
 *   Copyright (C) 2021 by Ronan Demoment                                  *
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

#ifndef NAVCOMPASSDRIVER_H_
#define NAVCOMPASSDRIVER_H_

/***************************************************************************/
/*                              Includes                                   */
/***************************************************************************/

#include <string>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

/***************************************************************************/
/*                                Types                                    */
/***************************************************************************/

struct vec
{
  float x, y, z;
};

using string = std::string;

/***************************************************************************/
/*                               Classes                                   */
/***************************************************************************/

class NavCompassDriver
{
public:
	virtual ~NavCompassDriver() = 0;
	virtual bool Init() = 0;
	virtual string GetDeviceName() = 0;
	virtual void GetMagneticField(vec *mag) = 0;
	virtual void GetAcceleration(vec *acc) = 0;};

#endif /* NAVCOMPASSDRIVER_H_ */
