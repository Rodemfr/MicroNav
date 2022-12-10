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

 /***************************************************************************/
 /*                              Includes                                   */
 /***************************************************************************/

#include "Configuration.h"
#include "BoardConfig.h"

#include <Arduino.h>
#include <EEPROM.h>
#include <CRC32.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

#define CONFIGURATION_EEPROM_SIZE 128
#define EEPROM_CONFIG_OFFSET      0
#define CONFIG_MAGIC_NUMBER       0x4D544E4D

/***************************************************************************/
/*                             Local types                                 */
/***************************************************************************/

#pragma pack(1)
typedef struct
{
	uint32_t magicWord;
	EEPROMConfig_t config;
	uint32_t checksum;
} ConfigBlock_t;
#pragma pack()

/***************************************************************************/
/*                           Local prototypes                              */
/***************************************************************************/

/***************************************************************************/
/*                               Globals                                   */
/***************************************************************************/

/***************************************************************************/
/*                              Functions                                  */
/***************************************************************************/

void Configuration::Init()
{
	EEPROM.begin(CONFIGURATION_EEPROM_SIZE);
}

Configuration::Configuration()
{
	// Set default configuration
	ram.navCompassAvailable = false;
	ram.displayAvailable = false;

	memset(&eeprom, 0, sizeof(eeprom));

	eeprom.windShift = 10;
	eeprom.deviceId = 0x03123456;
	eeprom.serialType = SERIAL_TYPE_BT;
}

Configuration::~Configuration()
{
}

void Configuration::LoadFromEeprom()
{
	ConfigBlock_t configBlock = { 0 };

	EEPROM.get(0, configBlock);
	uint8_t* pConfig = (uint8_t*)(&configBlock);

	if (configBlock.magicWord == CONFIG_MAGIC_NUMBER)
	{
		if (CRC32::calculate(pConfig, sizeof(ConfigBlock_t) - sizeof(uint32_t)) == configBlock.checksum)
		{
			eeprom = configBlock.config;
				eeprom.serialType = SERIAL_TYPE_BT;
			DeployConfiguration();
		}
	}
}

void Configuration::SaveToEeprom()
{
	ConfigBlock_t eepromBlock = { 0 };
	ConfigBlock_t configBlock = { 0 };

	uint8_t* pEepromBlock = (uint8_t*)(&eepromBlock);
	uint8_t* pConfig = (uint8_t*)(&configBlock);

	EEPROM.get(0, eepromBlock);

	configBlock.magicWord = CONFIG_MAGIC_NUMBER;
	configBlock.config = eeprom;

	configBlock.checksum = CRC32::calculate(pConfig, sizeof(ConfigBlock_t) - sizeof(uint32_t));

	for (uint32_t i = 0; i < sizeof(ConfigBlock_t); i++)
	{
		if (pEepromBlock[i] != pConfig[i])
		{
			EEPROM.put(0, configBlock);
			EEPROM.commit();
			break;
		}
	}
}

void Configuration::SaveCalibration(MicronetCodec& micronetCodec)
{
	eeprom.waterSpeedFactor_per = micronetCodec.navData.waterSpeedFactor_per;
	eeprom.waterTemperatureOffset_C = micronetCodec.navData.waterTemperatureOffset_degc;
	eeprom.depthOffset_m = micronetCodec.navData.depthOffset_m;
	eeprom.windSpeedFactor_per = micronetCodec.navData.windSpeedFactor_per;
	eeprom.windDirectionOffset_deg = micronetCodec.navData.windDirectionOffset_deg;
	eeprom.headingOffset_deg = micronetCodec.navData.headingOffset_deg;
	eeprom.magneticVariation_deg = micronetCodec.navData.magneticVariation_deg;
	eeprom.windShift = micronetCodec.navData.windShift_min;
	eeprom.timeZone_h = micronetCodec.navData.timeZone_h;

	SaveToEeprom();
}

void Configuration::LoadCalibration(MicronetCodec* micronetCodec)
{
	micronetCodec->navData.waterSpeedFactor_per = eeprom.waterSpeedFactor_per;
	micronetCodec->navData.waterTemperatureOffset_degc = eeprom.waterTemperatureOffset_C;
	micronetCodec->navData.depthOffset_m = eeprom.depthOffset_m;
	micronetCodec->navData.windSpeedFactor_per = eeprom.windSpeedFactor_per;
	micronetCodec->navData.windDirectionOffset_deg = eeprom.windDirectionOffset_deg;
	micronetCodec->navData.headingOffset_deg = eeprom.headingOffset_deg;
	micronetCodec->navData.magneticVariation_deg = eeprom.magneticVariation_deg;
	micronetCodec->navData.windShift_min = eeprom.windShift;
	micronetCodec->navData.timeZone_h = eeprom.timeZone_h;
}

void Configuration::DeployConfiguration()
{
	switch (eeprom.serialType)
	{
	case SERIAL_TYPE_USB:
		ram.nmeaLink = &Serial;
		break;
	case SERIAL_TYPE_BT:
		gBtSerial.begin(String("MicroNav"));
		ram.nmeaLink = &gBtSerial;
		break;
	case SERIAL_TYPE_WIFI:
		// TODO : Implement WiFi serial link
		ram.nmeaLink = &Serial;
		break;
	}
}