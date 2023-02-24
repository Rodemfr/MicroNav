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
// If we don't receive a request from a network for this time, we consider the network lost
#define NETWORK_LOST_TIME_MS 5000

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

/*
  Class constructor
*/
MicronetDevice::MicronetDevice(MicronetCodec *micronetCodec) : lastMasterSignalStrength(0), pingTimeStamp(0)
{
    memset(&deviceInfo, 0, sizeof(deviceInfo));
    this->micronetCodec = micronetCodec;
}

/*
  Class destructor
*/
MicronetDevice::~MicronetDevice()
{
}

/*
  Set the Device ID handled by MicronetDevice class.
  @param deviceId 32bit device ID
*/
void MicronetDevice::SetDeviceId(uint32_t deviceId)
{
    this->deviceInfo.deviceId = deviceId;
}

/*
  Set the Network ID attached to the micronet device.
  @param networkId 32bit network ID
*/
void MicronetDevice::SetNetworkId(uint32_t networkId)
{
    this->deviceInfo.networkId = networkId;
}

/*
  Set the data fields which must be sent by the Micronet device
  @param dataFields 32bit bitfield
*/
void MicronetDevice::SetDataFields(uint32_t dataFields)
{
    this->deviceInfo.dataFields = dataFields;

    // 
    SplitDataFields();
}

/*
  Add data fields to the list which must be sent by the Micronet device
  @param dataFields 32bit bitfield
*/
void MicronetDevice::AddDataFields(uint32_t dataFields)
{
    this->deviceInfo.dataFields |= dataFields;

    SplitDataFields();
}

/*
  Parse the incoming message, update the internal navigation data structure, and send back
  potential response message in the message FIFO.
  @param message Pointer to the micronet message
  @param messageFifo Pointer to the outgoing message queue
*/
void MicronetDevice::ProcessMessage(MicronetMessage_t *message, MicronetMessageFifo *messageFifo)
{
    TxSlotDesc_t      txSlot;
    MicronetMessage_t txMessage;
    // Check if the header CRC is valid. If not, the message is ignored and discarded.
    if (micronetCodec->VerifyHeaderCrc(message))
    {
        // Update the list of surrounding Micronet networks.
        UpdateNetworkScan(message);
        // Is the message addressed to our network ?
        if (micronetCodec->GetNetworkId(message) == deviceInfo.networkId)
        {
            // Yes : update the list of detected devices in the network
            UpdateDevicesInRange(message);

            // Is this the MASTER REQUEST message from the master device ?
            if (micronetCodec->GetMessageId(message) == MICRONET_MESSAGE_ID_MASTER_REQUEST)
            {
                // Yes : decode the network map from the message
                deviceInfo.state            = DEVICE_STATE_ACTIVE;
                deviceInfo.lastMasterCommMs = millis();
                micronetCodec->GetNetworkMap(message, &deviceInfo.networkMap);

                // Schedule the low power mode of CC1101 just at the end of the network cycle
                txMessage.action       = MICRONET_ACTION_RF_LOW_POWER;
                txMessage.startTime_us = micronetCodec->GetEndOfNetwork(&deviceInfo.networkMap);
                txMessage.len          = 0;
                messageFifo->Push(txMessage);

                // Schedule exit of CC1101's low power mode 1ms before actual start of the next network cycle.
                // It will let time for the PLL calibration loop to complete.
                txMessage.action       = MICRONET_ACTION_RF_ACTIVE_POWER;
                txMessage.startTime_us = micronetCodec->GetNextStartOfNetwork(&deviceInfo.networkMap) - 1000;
                txMessage.len          = 0;
                messageFifo->Push(txMessage);

                // Calculate signal strength of the MASTER REQUEST message. This strength will be transmitted in our outgoing messages to allow master
                // device to monitor the quality of the reception. It is used in the HEALTH page of Micronet displays.
                lastMasterSignalStrength = micronetCodec->CalculateSignalStrength(message);

                // For each virtual slave device...
                for (int i = 0; i < NUMBER_OF_VIRTUAL_DEVICES; i++)
                {
                    // Find the synchronous slot of the virtual device
                    txSlot = micronetCodec->GetSyncTransmissionSlot(&deviceInfo.networkMap, deviceInfo.deviceId + i);
                    if (txSlot.start_us != 0)
                    {
                        // Slot found : encode device data message
                        uint32_t payloadLength = micronetCodec->EncodeDataMessage(&txMessage, lastMasterSignalStrength, deviceInfo.networkId,
                                                                                  deviceInfo.deviceId + i, deviceInfo.splitDataFields[i]);
                        // Check that the sync slot is big enough for the encoded message
                        if (txSlot.payloadBytes < payloadLength)
                        {
                            // Sync slot is but too small : request slot resize
                            txSlot = micronetCodec->GetAsyncTransmissionSlot(&deviceInfo.networkMap);
                            micronetCodec->EncodeSlotUpdateMessage(&txMessage, lastMasterSignalStrength, deviceInfo.networkId,
                                                                   deviceInfo.deviceId + i, payloadLength);
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
                            // If we are here, it means we don't need the asynchronous slot. So we can use it to ping other devices to maintain a list
                            // of devices in range. We only ping when handling the first virtual slave device to avoid pinging too often.
                            if (i == 0)
                            {
                                PingNetwork(messageFifo);
                            }
                        }
                    }
                    else
                    {
                        // No synchronous slot available : request a slot
                        txSlot = micronetCodec->GetAsyncTransmissionSlot(&deviceInfo.networkMap);
                        micronetCodec->EncodeSlotRequestMessage(&txMessage, lastMasterSignalStrength, deviceInfo.networkId, deviceInfo.deviceId + i,
                                                                micronetCodec->GetDataMessageLength(deviceInfo.splitDataFields[i]));
                        txMessage.action       = MICRONET_ACTION_RF_TRANSMIT;
                        txMessage.startTime_us = txSlot.start_us;
                        messageFifo->Push(txMessage);
                    }
                }
            }
            else
            {
                // This is not the MASTER REQUEST message : decode it normally
                if (micronetCodec->DecodeMessage(message))
                {
                    // If DecodeMessage return value is true, then the received message requires an aknowledge in the asynchronous slot
                    for (int i = 0; i < NUMBER_OF_VIRTUAL_DEVICES; i++)
                    {
                        // Send a aknowledge for each of the virtual slave devices
                        txSlot = micronetCodec->GetAckTransmissionSlot(&deviceInfo.networkMap, deviceInfo.deviceId + i);
                        micronetCodec->EncodeAckParamMessage(&txMessage, lastMasterSignalStrength, deviceInfo.networkId, deviceInfo.deviceId + i);
                        txMessage.action       = MICRONET_ACTION_RF_TRANSMIT;
                        txMessage.startTime_us = txSlot.start_us;
                        messageFifo->Push(txMessage);
                    }
                }
            }
        }
    }

    // Remove inactive devices from network device list
    RemoveLostDevices();
    // Remove inactive networks from surrounding network list
    RemoveLostNetworks();
}

// Distribute requested data fields to the virtual devices
// This distribution is made to balance the size of the data message of each dvirtual evice
void MicronetDevice::SplitDataFields()
{
    // Clear current split data fields
    for (int i = 0; i < NUMBER_OF_VIRTUAL_DEVICES; i++)
    {
        deviceInfo.splitDataFields[i] = 0;
    }

    // Spread fields onto the virtual devices
    for (int i = 0; i < 32; i++)
    {
        if ((deviceInfo.dataFields >> i) & 0x1)
        {
            deviceInfo.splitDataFields[GetShortestDevice()] |= (1 << i);
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
        uint8_t size = micronetCodec->GetDataMessageLength(deviceInfo.splitDataFields[i]);
        if (size < minDeviceSize)
        {
            minDeviceSize  = size;
            minDeviceIndex = i;
        }
    }

    return minDeviceIndex;
}

DeviceInfo_t &MicronetDevice::GetDeviceInfo()
{
    return deviceInfo;
}

/*
  Update the list of devices of the attached network with the provided message
  @param message Message received on the attached network
*/
void MicronetDevice::UpdateDevicesInRange(MicronetMessage_t *message)
{
    bool     deviceFound = false;
    uint32_t deviceId    = micronetCodec->GetDeviceId(message);

    // Check each device of the list
    for (int i = 0; i < deviceInfo.nbDevicesInRange; i++)
    {
        // Is the received message device in the list ?
        if (deviceInfo.devicesInRange[i].deviceId == deviceId)
        {
            // Yes : update timestamp and signal strength
            deviceInfo.devicesInRange[i].lastCommMs       = millis();
            deviceInfo.devicesInRange[i].localRadioLevel  = micronetCodec->CalculateSignalStrength(message);
            deviceInfo.devicesInRange[i].remoteRadioLevel = micronetCodec->GetSignalStrength(message);
            deviceFound                                   = true;
            break;
        }
    }

    // Was the device already in the list ?
    if ((!deviceFound) && (deviceInfo.nbDevicesInRange < MICRONET_MAX_DEVICES_PER_NETWORK))
    {
        // No : add it
        deviceInfo.devicesInRange[deviceInfo.nbDevicesInRange].deviceId         = deviceId;
        deviceInfo.devicesInRange[deviceInfo.nbDevicesInRange].lastCommMs       = millis();
        deviceInfo.devicesInRange[deviceInfo.nbDevicesInRange].localRadioLevel  = micronetCodec->CalculateSignalStrength(message);
        deviceInfo.devicesInRange[deviceInfo.nbDevicesInRange].remoteRadioLevel = micronetCodec->GetSignalStrength(message);
        deviceInfo.nbDevicesInRange++;
    }
}

/*
  Update the list of surrounding networks with the provided message
  @param message Message received from any network
*/
void MicronetDevice::UpdateNetworkScan(MicronetMessage_t *message)
{
    bool     networkFound = false;
    uint32_t networkId    = micronetCodec->GetNetworkId(message);
    uint32_t now          = millis();

    // Only take into account MASTER REQUEST message. This is to avoid attaching to a network from which we only detect a close slave device and not
    // the master device.
    if (micronetCodec->GetMessageId(message) != MICRONET_MESSAGE_ID_MASTER_REQUEST)
    {
        return;
    }

    // Check each network of the list
    for (int i = 0; i < deviceInfo.nbNetworksInRange; i++)
    {
        // Is the network in the list ?
        if (deviceInfo.networksInRange[i].networkId == networkId)
        {
            // Yes : update timestamp and RSSI
            if ((message->rssi) > deviceInfo.networksInRange[i].rssi)
            {
                deviceInfo.networksInRange[i].rssi = message->rssi;
            }
            deviceInfo.networksInRange[i].timeStamp = now;
            networkFound                            = true;
            break;
        }
    }

    // Was the network already in the list ?
    if (!networkFound)
    {
        // No : add it
        if (deviceInfo.nbNetworksInRange < MAX_NETWORK_TO_SCAN)
        {
            // The list is not full : add the new network to the list
            deviceInfo.networksInRange[deviceInfo.nbNetworksInRange].networkId = networkId;
            deviceInfo.networksInRange[deviceInfo.nbNetworksInRange].timeStamp = now;
            deviceInfo.networksInRange[deviceInfo.nbNetworksInRange].rssi      = message->rssi;
            deviceInfo.nbNetworksInRange++;
        }
        else
        {
            // The list is full : replace the weakest one
            int16_t minRssi  = 1000;
            int     minIndex = 0;
            for (int i = 0; i < deviceInfo.nbNetworksInRange; i++)
            {
                if (deviceInfo.networksInRange[i].rssi < minRssi)
                {
                    minRssi  = deviceInfo.networksInRange[i].rssi;
                    minIndex = i;
                }
            }
            // Only replace if the new network is stronger
            if (minRssi < message->rssi)
            {
                deviceInfo.networksInRange[minIndex].networkId = networkId;
                deviceInfo.networksInRange[minIndex].timeStamp = now;
                deviceInfo.networksInRange[minIndex].rssi      = message->rssi;
            }
        }
    }
}

/*
  Remove lost devices from network device list.
*/
void MicronetDevice::RemoveLostDevices()
{
    uint32_t now = millis();

    // Check every device of the list
    for (int i = 0; i < deviceInfo.nbDevicesInRange; i++)
    {
        // Check last communication time of the device
        if ((now - deviceInfo.devicesInRange[i].lastCommMs) > DEVICE_LOST_TIME_MS)
        {
            // No news for too long : remove the device
            if (i < deviceInfo.nbDevicesInRange - 1)
            {
                memcpy(&deviceInfo.devicesInRange[i], &deviceInfo.devicesInRange[i + 1], sizeof(DeviceInfo_t) * deviceInfo.nbDevicesInRange - i - 1);
            }
            deviceInfo.nbDevicesInRange--;
        }
    }
}

/*
  Remove lost networks from surrounding network lists.
*/
void MicronetDevice::RemoveLostNetworks()
{
    uint32_t now = millis();

    // Check every network of the list
    for (int i = 0; i < deviceInfo.nbNetworksInRange; i++)
    {
        // Check last communication time of the network
        if ((now - deviceInfo.networksInRange[i].timeStamp) > NETWORK_LOST_TIME_MS)
        {
            // No news for too long : remove the network
            if (i < deviceInfo.nbNetworksInRange - 1)
            {
                memcpy(&deviceInfo.networksInRange[i], &deviceInfo.networksInRange[i + 1],
                       sizeof(DeviceInfo_t) * deviceInfo.nbNetworksInRange - i - 1);
            }
            deviceInfo.nbNetworksInRange--;
        }
    }
}

/*
    Create a PING message.
    @param messageFifo Pointer to the outgoing message FIFO
*/
void MicronetDevice::PingNetwork(MicronetMessageFifo *messageFifo)
{
    TxSlotDesc_t      txSlot;
    MicronetMessage_t txMessage;
    uint32_t          now = millis();

    // Only send the ping message every NETWORK_PING_PERIOD_MS to avoid flooding asynchronous slot
    if (now - pingTimeStamp > NETWORK_PING_PERIOD_MS)
    {
        pingTimeStamp = now;
        txSlot        = micronetCodec->GetAsyncTransmissionSlot(&deviceInfo.networkMap);
        micronetCodec->EncodePingMessage(&txMessage, 9, deviceInfo.networkMap.networkId, deviceInfo.deviceId);
        txMessage.action       = MICRONET_ACTION_RF_TRANSMIT;
        txMessage.startTime_us = txSlot.start_us;
        messageFifo->Push(txMessage);
    }
}

/*
  Update device internal status (lists, timeouts, states, etc.)
*/
void MicronetDevice::Yield()
{
    uint32_t now = millis();

    RemoveLostDevices();
    RemoveLostNetworks();

    // Check if network has been lost
    if ((deviceInfo.state == DEVICE_STATE_ACTIVE) && (now - deviceInfo.lastMasterCommMs > NETWORK_LOST_TIME_MS))
    {
        deviceInfo.state            = DEVICE_STATE_SEARCH_NETWORK;
        deviceInfo.nbDevicesInRange = 0;
    }
}