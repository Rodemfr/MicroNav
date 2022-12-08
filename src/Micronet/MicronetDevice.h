/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Implement Micronet Device behavior                            *
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

#ifndef MICRONETDEVICE_H_
#define MICRONETDEVICE_H_

/***************************************************************************/
/*                              Includes                                   */
/***************************************************************************/

#include "Micronet.h"
#include "MicronetCodec.h"
#include "MicronetMessageFifo.h"
#include <Arduino.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

#define NUMBER_OF_VIRTUAL_DEVICES 3

/***************************************************************************/
/*                                Types                                    */
/***************************************************************************/

typedef struct {
	uint32_t deviceId;
	uint8_t radioLevel;
	uint32_t lastCommMs;
} MicronetDeviceInfo_t;

typedef struct {
	bool connected;
	uint32_t deviceId;
	uint32_t networkId;
	MicronetCodec::NetworkMap networkMap;
	uint32_t lastMasterCommMs;
	uint32_t dataFields;
	uint32_t splitDataFields[NUMBER_OF_VIRTUAL_DEVICES];
	uint32_t nbDevicesInRange;
	MicronetDeviceInfo_t devicesInRange[MAX_DEVICES_PER_NETWORK];
} MicronetNetworkState_t;

/***************************************************************************/
/*                               Classes                                   */
/***************************************************************************/

class MicronetDevice
{
public:
	MicronetDevice(MicronetCodec *micronetCodec);
	virtual ~MicronetDevice();

	void SetDeviceId(uint32_t deviceId);
	void SetNetworkId(uint32_t networkId);
	void SetDataFields(uint32_t dataMask);
	void AddDataFields(uint32_t dataMask);
	void ProcessMessage(MicronetMessage_t *message, MicronetMessageFifo *messageFifo);
	MicronetNetworkState_t &GetNetworkStatus();
	void Yield();

private:
	MicronetCodec *micronetCodec;
	uint8_t latestSignalStrength;
	MicronetNetworkState_t networkState;
	uint32_t pingTimeStamp;

	void SplitDataFields();
	uint8_t GetShortestDevice();
	void UpdateDevicesInRange(MicronetMessage_t *message);
	void RemoveLostDevices();
	void PingNetwork(MicronetMessageFifo* messageFifo);
};

#endif /* MICRONETDEVICE_H_ */
