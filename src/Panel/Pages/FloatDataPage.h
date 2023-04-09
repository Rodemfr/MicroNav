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

#ifndef FLOATDATAPAGE_H_
#define FLOATDATAPAGE_H_

/***************************************************************************/
/*                              Includes                                   */
/***************************************************************************/

#include "NavigationData.h"
#include "PageHandler.h"

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

/***************************************************************************/
/*                                Types                                    */
/***************************************************************************/

/***************************************************************************/
/*                               Classes                                   */
/***************************************************************************/

class FloatDataPage : public PageHandler
{
  public:
    FloatDataPage();
    virtual ~FloatDataPage();

    bool Draw(bool force, bool flushDisplay = true);
    void SetData(FloatValue_t *value, std::string unit, bool fracDigit);
    void SetData(FloatValue_t *value1, std::string unit1, bool fracDigit1, FloatValue_t *value2, std::string unit2, bool fracDigit2);

  private:
    FloatValue_t *value1;
    FloatValue_t *value2;
    std::string unit1;
    std::string unit2;
    bool fracDigit1;
    bool fracDigit2;
    int32_t prevValue1, prevValue2;
    bool prevValue1Valid, prevValue2Valid;
};

#endif
