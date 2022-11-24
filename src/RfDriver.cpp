/***************************************************************************
 *                                                                         *
 * Project:  MicronetToNMEA                                                *
 * Purpose:  Driver for CC1101                                             *
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

const uint8_t RfDriver::preambleAndSync[MICRONET_RF_PREAMBLE_LENGTH] = {
    MICRONET_RF_PREAMBLE_BYTE, MICRONET_RF_PREAMBLE_BYTE,
    MICRONET_RF_PREAMBLE_BYTE, MICRONET_RF_PREAMBLE_BYTE,
    MICRONET_RF_PREAMBLE_BYTE, MICRONET_RF_PREAMBLE_BYTE,
    MICRONET_RF_PREAMBLE_BYTE, MICRONET_RF_PREAMBLE_BYTE,
    MICRONET_RF_PREAMBLE_BYTE, MICRONET_RF_PREAMBLE_BYTE,
    MICRONET_RF_PREAMBLE_BYTE, MICRONET_RF_PREAMBLE_BYTE,
    MICRONET_RF_PREAMBLE_BYTE, MICRONET_RF_PREAMBLE_BYTE,
    MICRONET_RF_SYNC_BYTE};

/***************************************************************************/
/*                              Functions                                  */
/***************************************************************************/

RfDriver::RfDriver()
    : messageFifo(nullptr), rfState(RF_STATE_RX_WAIT_SYNC),
      nextTransmitIndex(-1), messageBytesSent(0), frequencyOffset_MHz(0),
      freqTrackingNID(0), cpuFreq_MHz(0), txTimer(nullptr)
{
  memset(transmitList, 0, sizeof(transmitList));
}

RfDriver::~RfDriver() {}

bool RfDriver::Init(MicronetMessageFifo *messageFifo,
                    float frequencyOffset_mHz)
{
  this->frequencyOffset_MHz = frequencyOffset_mHz;
  this->messageFifo = messageFifo;
  rfDriver = this;

  cpuFreq_MHz = getCpuFrequencyMhz();
  txTimer = timerBegin(0, cpuFreq_MHz, true);
  ;
  timerAttachInterrupt(txTimer, TimerHandler, true);

  if (!sx1276Driver.Init(RF_SCK_PIN, RF_MOSI_PIN, RF_MISO_PIN, RF_CS0_PIN, RF_DIO0_PIN, RF_DIO1_PIN, RF_RST_PIN, messageFifo))
  {
    return false;
  }

  sx1276Driver.SetFrequency(MICRONET_RF_CENTER_FREQUENCY_MHZ + frequencyOffset_MHz);
  SetBandwidth(RF_BANDWIDTH_HIGH);

  return true;
}

void RfDriver::Start()
{
  sx1276Driver.StartRx();
}

void RfDriver::SetFrequencyOffset(float offset_MHz)
{
  frequencyOffset_MHz = offset_MHz;
}

void RfDriver::SetFrequency(float frequency_MHz)
{
  sx1276Driver.GoToIdle();
  sx1276Driver.SetFrequency(frequency_MHz + frequencyOffset_MHz);
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

void RfDriver::RfIsr_Tx()
{
  if (rfState == RF_STATE_TX_TRANSMIT)
  {
    int bytesInFifo;
    int bytesToLoad = transmitList[nextTransmitIndex].len - messageBytesSent;

    // FIXME : handle FIFO correctly
    bytesInFifo = 13;
    if (bytesToLoad + bytesInFifo > SX1276_FIFO_MAX_SIZE)
    {
      bytesToLoad = SX1276_FIFO_MAX_SIZE - bytesInFifo;
    }

    // sx1276Driver.WriteFifo(
    //     &transmitList[nextTransmitIndex].data[messageBytesSent], bytesToLoad);
    // messageBytesSent += bytesToLoad;

    // if (messageBytesSent >= transmitList[nextTransmitIndex].len)
    // {
    //   rfState = RF_STATE_TX_LAST_TRANSMIT;
    //   sx1276Driver.IrqOnTxFifoUnderflow();
    // }
  }
  else
  {
    transmitList[nextTransmitIndex].startTime_us = 0;
    nextTransmitIndex = -1;

    ScheduleTransmit();
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
  noInterrupts();
  int transmitIndex = GetFreeTransmitSlot();

  if (transmitIndex >= 0)
  {
    transmitList[transmitIndex].action = message->action;
    transmitList[transmitIndex].startTime_us = message->startTime_us;
    transmitList[transmitIndex].len = message->len;
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
  interrupts();
}

void RfDriver::ScheduleTransmit()
{
  do
  {
    int transmitIndex = GetNextTransmitIndex();
    if (transmitIndex < 0)
    {
      // No transmit to schedule : stop timer and leave
      //			timerInt.stop();
      return;
    }
    int32_t transmitDelay = transmitList[transmitIndex].startTime_us - micros();
    if ((transmitDelay <= 0) || (transmitDelay > 3000000))
    {
      // Transmit already in the past, or invalid : delete it and schedule the
      // next one
      transmitList[transmitIndex].startTime_us = 0;
      continue;
    }

    // Schedule new transmit
    nextTransmitIndex = transmitIndex;
    //		timerInt.trigger(transmitDelay);

    return;
  } while (true);
}

int RfDriver::GetNextTransmitIndex()
{
  uint32_t minTime = 0xffffffff;
  int minIndex = -1;

  for (int i = 0; i < TRANSMIT_LIST_SIZE; i++)
  {
    if (transmitList[i].startTime_us != 0)
    {
      if (transmitList[i].startTime_us <= minTime)
      {
        minTime = transmitList[i].startTime_us;
        minIndex = i;
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
    if (transmitList[i].startTime_us == 0)
    {
      freeIndex = i;
      break;
    }
  }

  return freeIndex;
}

void RfDriver::TimerHandler() { rfDriver->TransmitCallback(); }

void RfDriver::TransmitCallback()
{
  if (nextTransmitIndex < 0)
  {
    return;
  }

  int32_t triggerDelay =
      micros() - transmitList[nextTransmitIndex].startTime_us;

  if (triggerDelay < 0)
  {
    // Depending on the Teensy version, timer may not be able to reach delay of
    // more than about 50ms. in that case we reprogram it until we reach the
    // specified amount of time
    ScheduleTransmit();
    return;
  }

  if (transmitList[nextTransmitIndex].action == MICRONET_ACTION_RF_LOW_POWER)
  {
    transmitList[nextTransmitIndex].startTime_us = 0;
    nextTransmitIndex = -1;

    sx1276Driver.LowPower();
    rfState = RF_STATE_RX_WAIT_SYNC;

    ScheduleTransmit();
  }
  else if (transmitList[nextTransmitIndex].action ==
           MICRONET_ACTION_RF_ACTIVE_POWER)
  {
    transmitList[nextTransmitIndex].startTime_us = 0;
    nextTransmitIndex = -1;

    sx1276Driver.ActivePower();
    ScheduleTransmit();
  }
  else if (rfState == RF_STATE_RX_WAIT_SYNC)
  {
    rfState = RF_STATE_TX_TRANSMIT;

    // Change CC1101 configuration for transmission
    sx1276Driver.GoToIdle();
    // sx1276Driver.IrqOnTxFifoThreshold();
    // sx1276Driver.SetFifoThreshold(8);

    // Fill FIFO with preamble's first byte
    // sx1276Driver.WriteFifo(MICRONET_RF_PREAMBLE_BYTE);

    // Start transmission as soon as we have the first byte available in FIFO to
    // minimize latency
    sx1276Driver.StartTx();

    // Fill FIFO with rest of preamble and sync byte
    // sx1276Driver.WriteFifo(static_cast<const uint8_t *>(preambleAndSync),
    //                        sizeof(preambleAndSync));

    messageBytesSent = 0;
  }
  else
  {
    ScheduleTransmit();
  }
}

void RfDriver::EnableFrequencyTracking(uint32_t networkId)
{
  freqTrackingNID = networkId;
}

void RfDriver::DisableFrequencyTracking() { freqTrackingNID = 0; }
