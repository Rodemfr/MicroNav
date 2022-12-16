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

/***************************************************************************/
/*                              Includes                                   */
/***************************************************************************/

#include "MicronetDevice.h"

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

// Time in millisecond between two pings of the network
#define NETWORK_PING_PERIOD_MS 20000
// If a device does not communicate for this time, we consider it lost
#define DEVICE_LOST_TIME_MS 60000
// If we don't receive a request from the master for this time, we consider the network lost
#define NETWORK_LOST_TIME_MS 10000

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

MicronetDevice::MicronetDevice(MicronetCodec *micronetCodec) : latestSignalStrength(0), pingTimeStamp(0)
{
    memset(&networkState, 0, sizeof(networkState));
    this->micronetCodec = micronetCodec;
}

MicronetDevice::~MicronetDevice()
{
}

void MicronetDevice::SetDeviceId(uint32_t deviceId)
{
    this->networkState.deviceId = deviceId;
}

void MicronetDevice::SetNetworkId(uint32_t networkId)
{
    this->networkState.networkId = networkId;
}

void MicronetDevice::SetDataFields(uint32_t dataFields)
{
    this->networkState.dataFields = dataFields;

    SplitDataFields();
}

void MicronetDevice::AddDataFields(uint32_t dataFields)
{
    this->networkState.dataFields |= dataFields;

    SplitDataFields();
}

void MicronetDevice::ProcessMessage(MicronetMessage_t *message, MicronetMessageFifo *messageFifo)
{
    TxSlotDesc_t      txSlot;
    MicronetMessage_t txMessage;

    // Is the message addresses to us ?
    if ((micronetCodec->GetNetworkId(message) == networkState.networkId) && (micronetCodec->VerifyHeaderCrc(message)))
    {
        UpdateDevicesInRange(message);

        if (micronetCodec->GetMessageId(message) == MICRONET_MESSAGE_ID_MASTER_REQUEST)
        {
            networkState.connected        = true;
            networkState.lastMasterCommMs = millis();
            micronetCodec->GetNetworkMap(message, &networkState.networkMap);

            // We schedule the low power mode of CC1101 just at the end of the network cycle
            txMessage.action       = MICRONET_ACTION_RF_LOW_POWER;
            txMessage.startTime_us = micronetCodec->GetEndOfNetwork(&networkState.networkMap);
            txMessage.len          = 0;
            messageFifo->Push(txMessage);

            // We schedule exit of CC1101's low power mode 1ms before actual start of the next network cycle.
            // It will let time for the PLL calibration loop to complete.
            txMessage.action       = MICRONET_ACTION_RF_ACTIVE_POWER;
            txMessage.startTime_us = micronetCodec->GetNextStartOfNetwork(&networkState.networkMap) - 1000;
            txMessage.len          = 0;
            messageFifo->Push(txMessage);

            latestSignalStrength = micronetCodec->CalculateSignalStrength(message);

            for (int i = 0; i < NUMBER_OF_VIRTUAL_DEVICES; i++)
            {
                txSlot = micronetCodec->GetSyncTransmissionSlot(&networkState.networkMap, networkState.deviceId + i);
                if (txSlot.start_us != 0)
                {
                    uint32_t payloadLength = micronetCodec->EncodeDataMessage(&txMessage, latestSignalStrength, networkState.networkId,
                                                                              networkState.deviceId + i, networkState.splitDataFields[i]);
                    if (txSlot.payloadBytes < payloadLength)
                    {
                        // Sync slot is available but too small : request slot resize
                        txSlot = micronetCodec->GetAsyncTransmissionSlot(&networkState.networkMap);
                        micronetCodec->EncodeSlotUpdateMessage(&txMessage, latestSignalStrength, networkState.networkId, networkState.deviceId + i,
                                                               payloadLength);
                        txMessage.action       = MICRONET_ACTION_RF_TRANSMIT;
                        txMessage.startTime_us = txSlot.start_us;
                        messageFifo->Push(txMessage);
                    }
                    else
                    {
                        // Sync slot is ok : transmit message
                        txMessage.action       = MICRONET_ACTION_RF_TRANSMIT;
                        txMessage.startTime_us = txSlot.start_us;
                        messageFifo->Push(txMessage);
                        // Also ping other devices to maintain a list of devices in range
                        PingNetwork(messageFifo);
                    }
                }
                else
                {
                    // No sync slot available : request a slot
                    txSlot = micronetCodec->GetAsyncTransmissionSlot(&networkState.networkMap);
                    micronetCodec->EncodeSlotRequestMessage(&txMessage, latestSignalStrength, networkState.networkId, networkState.deviceId + i,
                                                            micronetCodec->GetDataMessageLength(networkState.splitDataFields[i]));
                    txMessage.action       = MICRONET_ACTION_RF_TRANSMIT;
                    txMessage.startTime_us = txSlot.start_us;
                    messageFifo->Push(txMessage);
                }
            }
        }
        else
        {
            if (micronetCodec->DecodeMessage(message))
            {
                for (int i = 0; i < NUMBER_OF_VIRTUAL_DEVICES; i++)
                {
                    txSlot = micronetCodec->GetAckTransmissionSlot(&networkState.networkMap, networkState.deviceId + i);
                    micronetCodec->EncodeAckParamMessage(&txMessage, latestSignalStrength, networkState.networkId, networkState.deviceId + i);
                    txMessage.action       = MICRONET_ACTION_RF_TRANSMIT;
                    txMessage.startTime_us = txSlot.start_us;
                    messageFifo->Push(txMessage);
                }
            }
        }
    }

    RemoveLostDevices();
}

// Distribute requested data fields to the virtual devices
// This distribution is made to balance the size of the data message of each dvirtual evice
void MicronetDevice::SplitDataFields()
{
    // Clear current split data fields
    for (int i = 0; i < NUMBER_OF_VIRTUAL_DEVICES; i++)
    {
        networkState.splitDataFields[i] = 0;
    }

    // Spread fields onto the virtual devices
    for (int i = 0; i < 32; i++)
    {
        if ((networkState.dataFields >> i) & 0x1)
        {
            networkState.splitDataFields[GetShortestDevice()] |= (1 << i);
        }
    }
}

// Returns the index of the virtual device with the shortest data message
uint8_t MicronetDevice::GetShortestDevice()
{
    uint8_t  minDeviceSize  = 255;
    uint32_t minDeviceIndex = 0;

    for (int i = 0; i < NUMBER_OF_VIRTUAL_DEVICES; i++)
    {
        uint8_t size = micronetCodec->GetDataMessageLength(networkState.splitDataFields[i]);
        if (size < minDeviceSize)
        {
            minDeviceSize  = size;
            minDeviceIndex = i;
        }
    }

    return minDeviceIndex;
}

MicronetNetworkState_t &MicronetDevice::GetNetworkStatus()
{
    return networkState;
}

void MicronetDevice::UpdateDevicesInRange(MicronetMessage_t *message)
{
    bool     deviceFound = false;
    uint32_t deviceId    = micronetCodec->GetDeviceId(message);

    for (int i = 0; i < networkState.nbDevicesInRange; i++)
    {
        if (networkState.devicesInRange[i].deviceId == deviceId)
        {
            networkState.devicesInRange[i].deviceId   = deviceId;
            networkState.devicesInRange[i].lastCommMs = millis();
            networkState.devicesInRange[i].radioLevel = micronetCodec->CalculateSignalStrength(message);
            deviceFound                               = true;
            break;
        }
    }

    if ((!deviceFound) && (networkState.nbDevicesInRange < MICRONET_MAX_DEVICES_PER_NETWORK))
    {
        networkState.devicesInRange[networkState.nbDevicesInRange].deviceId   = deviceId;
        networkState.devicesInRange[networkState.nbDevicesInRange].lastCommMs = millis();
        networkState.devicesInRange[networkState.nbDevicesInRange].radioLevel = micronetCodec->CalculateSignalStrength(message);
        networkState.nbDevicesInRange++;
    }
}

void MicronetDevice::RemoveLostDevices()
{
    uint32_t now = millis();

    for (int i = 0; i < networkState.nbDevicesInRange; i++)
    {
        if ((now - networkState.devicesInRange[i].lastCommMs) > DEVICE_LOST_TIME_MS)
        {
            if (i < networkState.nbDevicesInRange - 1)
            {
                memcpy(&networkState.devicesInRange[i], &networkState.devicesInRange[i + 1],
                       sizeof(MicronetDeviceInfo_t) * networkState.nbDevicesInRange - i - 1);
            }
            networkState.nbDevicesInRange--;
        }
    }
}

void MicronetDevice::PingNetwork(MicronetMessageFifo *messageFifo)
{
    TxSlotDesc_t      txSlot;
    MicronetMessage_t txMessage;
    uint32_t          now = millis();

    if (now - pingTimeStamp > NETWORK_PING_PERIOD_MS)
    {
        pingTimeStamp = now;
        txSlot        = micronetCodec->GetAsyncTransmissionSlot(&networkState.networkMap);
        micronetCodec->EncodePingMessage(&txMessage, 9, networkState.networkMap.networkId, networkState.deviceId);
        txMessage.action       = MICRONET_ACTION_RF_TRANSMIT;
        txMessage.startTime_us = txSlot.start_us;
        messageFifo->Push(txMessage);
    }
}

void MicronetDevice::Yield()
{
    uint32_t now = millis();

    RemoveLostDevices();
    if (now - networkState.lastMasterCommMs > NETWORK_LOST_TIME_MS)
    {
        networkState.connected        = false;
        networkState.nbDevicesInRange = 0;
    }
}