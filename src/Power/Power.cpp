/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Power Manager                                                 *
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

/***************************************************************************/
/*                              Includes                                   */
/***************************************************************************/

#include "Power.h"

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
/*                               Globals                                   */
/***************************************************************************/

/***************************************************************************/
/*                              Functions                                  */
/***************************************************************************/

Power::Power()
{
}

Power::~Power()
{
}

bool Power::Init()
{
    bool initStatus = false;

    if (!AXPDriver.begin(Wire, AXP192_SLAVE_ADDRESS))
    {
        AXPDriver.setPowerOutPut(AXP192_LDO2, AXP202_ON); // RF
        AXPDriver.setPowerOutPut(AXP192_LDO3, AXP202_ON); // GPS
        AXPDriver.setPowerOutPut(AXP192_DCDC2, AXP202_ON);
        AXPDriver.setPowerOutPut(AXP192_EXTEN, AXP202_ON);
        AXPDriver.setPowerOutPut(AXP192_DCDC1, AXP202_ON); // OLED

        initStatus = true;
    }

    return initStatus;
}

void Power::Shutdown()
{
    AXPDriver.shutdown();
}