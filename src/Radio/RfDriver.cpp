/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Scheduler of RF communications                                *
 * Author:   Ronan Demoment heavily based on ELECHOUSE's driver            *
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

#include "RfDriver.h"
#include "BoardConfig.h"
#include "Globals.h"
#include "Micronet.h"
#include "SX1276MnetDriver.h"

#include <Arduino.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

#define TX_DELAY_COMPENSATION 200

/***************************************************************************/
/*                             Local types                                 */
/***************************************************************************/

/***************************************************************************/
/*                           Local prototypes                              */
/***************************************************************************/

/***************************************************************************/
/*                               Globals                                   */
/***************************************************************************/

RfDriver *RfDriver::rfDriver;

/***************************************************************************/
/*                              Functions                                  */
/***************************************************************************/

RfDriver::RfDriver() : messageFifo(nullptr), nextTransmitIndex(-1), messageBytesSent(0), freqTrackingNID(0), txTimer(nullptr)
{
    timerMux = portMUX_INITIALIZER_UNLOCKED;
    memset((void *)transmitList, 0, sizeof(transmitList));
}

RfDriver::~RfDriver()
{
}

bool RfDriver::Init(MicronetMessageFifo *messageFifo)
{
    this->messageFifo = messageFifo;
    rfDriver          = this;

    txTimer = timerBegin(0, getApbFrequency() / 1000000, true);
    timerAlarmDisable(txTimer);
    timerAttachInterrupt(txTimer, TimerHandler, true);

    if (!sx1276Driver.Init(RF_SCK_PIN, RF_MOSI_PIN, RF_MISO_PIN, RF_CS0_PIN, RF_DIO0_PIN, RF_DIO1_PIN, RF_RST_PIN, messageFifo))
    {
        return false;
    }

    if (gConfiguration.eeprom.freqSystem == RF_FREQ_SYSTEM_868)
    {
        sx1276Driver.SetFrequency(MICRONET_RF_CENTER_FREQUENCY_868MHZ);
    }
    else
    {
        sx1276Driver.SetFrequency(MICRONET_RF_CENTER_FREQUENCY_915MHZ);
        SetBandwidth(RF_BANDWIDTH_HIGH);
    }

    return true;
}

void RfDriver::Start()
{
    sx1276Driver.StartRx();
}

void RfDriver::SetFrequency(float frequency_MHz)
{
    sx1276Driver.GoToIdle();
    sx1276Driver.SetFrequency(frequency_MHz);
    sx1276Driver.StartRx();
}

void RfDriver::SetBandwidth(RfBandwidth_t bandwidth)
{
    switch (bandwidth)
    {
    case RF_BANDWIDTH_LOW:
        sx1276Driver.SetBandwidth(95);
        break;
    case RF_BANDWIDTH_MEDIUM:
        sx1276Driver.SetBandwidth(125);
        break;
    default:
        sx1276Driver.SetBandwidth(250);
        break;
    }
}

void RfDriver::Transmit(MicronetMessageFifo *txMessageFifo)
{
    MicronetMessage_t *txMessage;
    while ((txMessage = txMessageFifo->Peek()) != nullptr)
    {
        Transmit(txMessage);
        txMessageFifo->DeleteMessage();
    }
}

void RfDriver::Transmit(MicronetMessage_t *message)
{
    taskENTER_CRITICAL(&timerMux);
    int transmitIndex = GetFreeTransmitSlot();

    if (transmitIndex >= 0)
    {
        transmitList[transmitIndex].action       = message->action;
        transmitList[transmitIndex].startTime_us = message->startTime_us;
        transmitList[transmitIndex].len          = message->len;
        if ((message->len > 0) && (message->len <= MICRONET_MAX_MESSAGE_LENGTH))
        {
            memcpy(transmitList[transmitIndex].data, message->data, message->len);
        }
        else
        {
            message->len = 0;
        }
    }

    ScheduleTransmit();
    taskEXIT_CRITICAL(&timerMux);
}

void RfDriver::ScheduleTransmit()
{
    do
    {
        int transmitIndex = GetNextTransmitIndex();
        if ((transmitIndex < 0) || (transmitIndex == nextTransmitIndex))
        {
            // No new transmit to schedule : leave
            nextTransmitIndex = transmitIndex;
            return;
        }

        // A new transmit is to be scheduled : stop current timer
        timerStop(txTimer);
        timerAlarmDisable(txTimer);
        timerWrite(txTimer, 0);

        // Check that we are not already late for this transmit
        uint32_t now           = micros();
        uint32_t transmitDelay = transmitList[transmitIndex].startTime_us - now - TX_DELAY_COMPENSATION;
        if (transmitDelay > 60000000)
        {
            // transmitDelay is more than 1 minute away, this is invalid
            transmitList[transmitIndex].action = MICRONET_ACTION_RF_NO_ACTION;
            continue;
        }

        // Transmit is valid : program the timer accordingly
        nextTransmitIndex = transmitIndex;
        timerAlarmWrite(txTimer, transmitDelay, false);
        timerAlarmEnable(txTimer);
        timerStart(txTimer);
        return;

    } while (true);
}

int RfDriver::GetNextTransmitIndex()
{
    uint32_t minTime         = 0xffffffff;
    int      minIndex        = -1;
    bool     upperTimestamps = false;
    bool     lowerTimestamps = false;
    bool     timeWrap        = false;

    for (int i = 0; i < TRANSMIT_LIST_SIZE; i++)
    {
        if (transmitList[i].action != MICRONET_ACTION_RF_NO_ACTION)
        {
            if (transmitList[i].startTime_us < 0x80000000UL)
            {
                lowerTimestamps = true;
            }
            else
            {
                upperTimestamps = true;
            }
        }
    }

    timeWrap = lowerTimestamps && upperTimestamps;

    for (int i = 0; i < TRANSMIT_LIST_SIZE; i++)
    {
        if (transmitList[i].action != MICRONET_ACTION_RF_NO_ACTION)
        {
            if ((!timeWrap) || (transmitList[i].startTime_us > 0x80000000UL))
            {
                if (transmitList[i].startTime_us <= minTime)
                {
                    minTime  = transmitList[i].startTime_us;
                    minIndex = i;
                }
            }
        }
    }

    return minIndex;
}

int RfDriver::GetFreeTransmitSlot()
{
    int freeIndex = -1;

    for (int i = 0; i < TRANSMIT_LIST_SIZE; i++)
    {
        if (transmitList[i].action == MICRONET_ACTION_RF_NO_ACTION)
        {
            freeIndex = i;
            break;
        }
    }

    return freeIndex;
}

void RfDriver::TimerHandler()
{
    rfDriver->TransmitCallback();
}

void RfDriver::TransmitCallback()
{
    portENTER_CRITICAL_ISR(&timerMux);

    if (nextTransmitIndex < 0)
    {
        portEXIT_CRITICAL_ISR(&timerMux);
        return;
    }

    sx1276Driver.TransmitFromIsr(transmitList[nextTransmitIndex]);
    transmitList[nextTransmitIndex].action = MICRONET_ACTION_RF_NO_ACTION;
    ScheduleTransmit();
    portEXIT_CRITICAL_ISR(&timerMux);
}

void RfDriver::EnableFrequencyTracking(uint32_t networkId)
{
    freqTrackingNID = networkId;
}

void RfDriver::DisableFrequencyTracking()
{
    freqTrackingNID = 0;
}
