/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Configuration handler                                         *
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

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

/***************************************************************************/
/*                              Includes                                   */
/***************************************************************************/

#include "MicronetCodec.h"

#include <stdint.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

/***************************************************************************/
/*                                Types                                    */
/***************************************************************************/

typedef enum {
	SERIAL_TYPE_USB = 0,
	SERIAL_TYPE_WIRED,
	SERIAL_TYPE_BT,
	SERIAL_TYPE_WIFI
} SerialType_t;

/***************************************************************************/
/*                               Classes                                   */
/***************************************************************************/

class Configuration
{
public:
	Configuration();
	virtual ~Configuration();

	void Init();
	void LoadFromEeprom();
	void SaveToEeprom();
	void SaveCalibration(MicronetCodec& micronetCodec);
	void LoadCalibration(MicronetCodec* micronetCodec);

	// The following parameters are NOT loaded/saved from/to EEPROM
	bool navCompassAvailable;
	bool displayAvailable;
	SerialType_t serialType;

	// The following parameters are loaded/saved from/to EEPROM
	uint32_t networkId;
	uint32_t deviceId;
	float waterSpeedFactor_per;
	float waterTemperatureOffset_C;
	float depthOffset_m;
	float windSpeedFactor_per;
	float windDirectionOffset_deg;
	float headingOffset_deg;
	float magneticVariation_deg;
	float windShift;
	float xMagOffset;
	float yMagOffset;
	float zMagOffset;
	float rfFrequencyOffset_MHz;
	int8_t timeZone_h;
};

/***************************************************************************/
/*                              Prototypes                                 */
/***************************************************************************/

#endif /* CONFIGURATION_H_ */
