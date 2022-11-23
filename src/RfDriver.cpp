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
      freqTrackingNID(0)
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

  // timerInt.begin(TimerHandler);

  if (!sx1276Driver.Init(RF_SCK_PIN, RF_MOSI_PIN, RF_MISO_PIN, RF_CS0_PIN,
                         RF_DIO0_PIN, RF_DIO1_PIN, RF_RST_PIN, messageFifo))
  {
    return false;
  }

  sx1276Driver.SetFrequency(MICRONET_RF_CENTER_FREQUENCY_MHZ +
                            frequencyOffset_mHz);
  sx1276Driver.SetDeviation(MICRONET_RF_DEVIATION_KHZ);
  sx1276Driver.SetBitrate(MICRONET_RF_BAUDRATE_BAUD / 1000.0f);
  sx1276Driver.SetBandwidth(250);
  sx1276Driver.SetSyncByte(0x99);
  sx1276Driver.SetPacketLength(SX1276_FIFO_MAX_SIZE);

  return true;
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

void RfDriver::RfIsr()
{
  if ((rfState == RF_STATE_TX_TRANSMIT) ||
      (rfState == RF_STATE_TX_LAST_TRANSMIT))
  {
    RfIsr_Tx();
  }
  else
  {
    RfIsr_Rx();
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

    sx1276Driver.WriteFifo(
        &transmitList[nextTransmitIndex].data[messageBytesSent], bytesToLoad);
    messageBytesSent += bytesToLoad;

    if (messageBytesSent >= transmitList[nextTransmitIndex].len)
    {
      rfState = RF_STATE_TX_LAST_TRANSMIT;
      sx1276Driver.IrqOnTxFifoUnderflow();
    }
  }
  else
  {
    transmitList[nextTransmitIndex].startTime_us = 0;
    nextTransmitIndex = -1;

    RestartReception();
    ScheduleTransmit();
  }
}

void RfDriver::RfIsr_Rx()
{
  static MicronetMessage_t message;
  static int dataOffset;
  static int packetLength;
  static uint32_t startTime_us;
  uint8_t nbBytes;

  if (rfState == RF_STATE_RX_WAIT_SYNC)
  {
    // When we reach this point, we know that a packet is under reception by
    // CC1101. We will not wait the end of this reception and will begin
    // collecting bytes right now. This way we will be able to receive packets
    // that are longer than FIFO size and we will instruct CC1101 to change
    // packet size on the fly as soon as we will have identified the length
    // field
    rfState = RF_STATE_RX_HEADER;
    startTime_us = micros() - PREAMBLE_LENGTH_IN_US;
    packetLength = -1;
    dataOffset = 0;

    // FIXME : How many bytes are already in the FIFO ?
    nbBytes = 13; // sx1276Driver.GetRxFifoLevel();

    // Check for FIFO overflow
    if (nbBytes > 64)
    {
      // Yes : ignore current packet and restart CC1101 reception for the next
      // packet
      RestartReception();
      return;
    }

    startTime_us -= nbBytes * BYTE_LENGTH_IN_US;
  }
  else if ((rfState == RF_STATE_RX_HEADER) ||
           (rfState == RF_STATE_RX_PAYLOAD))
  {
    // FIXME : How many bytes ?
    nbBytes = 13; // sx1276Driver.GetRxFifoLevel();

    // Check for FIFO overflow
    if (nbBytes > 64)
    {
      // Yes : ignore current packet and restart CC1101 reception for the next
      // packet
      RestartReception();
      return;
    }
  }
  else
  {
    // GDO0 RX ISR is not supposed to be triggered in this state
    return;
  }

  // Are there new bytes in the FIFO ?
  if ((nbBytes > 0) && ((dataOffset < packetLength) || (packetLength < 0)))
  {
    // Yes : read them
    if (dataOffset + nbBytes > MICRONET_MAX_MESSAGE_LENGTH)
    {
      // Received data size exceeds max message size, something must have gone
      // wrong : restart listening
      RestartReception();
      return;
    }
    sx1276Driver.ReadFifo(message.data + dataOffset, nbBytes);
    dataOffset += nbBytes;
    // Check if we have reached the packet length field
    if ((rfState == RF_STATE_RX_HEADER) &&
        (dataOffset >= (MICRONET_LEN_OFFSET_1 + 2)))
    {
      rfState = RF_STATE_RX_PAYLOAD;
      // Yes : check that this is a valid length
      if ((message.data[MICRONET_LEN_OFFSET_1] ==
           message.data[MICRONET_LEN_OFFSET_2]) &&
          (message.data[MICRONET_LEN_OFFSET_1] <
           MICRONET_MAX_MESSAGE_LENGTH - 3) &&
          ((message.data[MICRONET_LEN_OFFSET_1] + 2) >=
           MICRONET_PAYLOAD_OFFSET))
      {
        packetLength = message.data[MICRONET_LEN_OFFSET_1] + 2;
        // Update CC1101's packet length register
        sx1276Driver.SetPacketLength(packetLength);
      }
      else
      {
        // The packet length is not valid : ignore current packet and restart
        // CC1101 reception for the next packet
        RestartReception();
        return;
      }
    }
  }

  if ((dataOffset < packetLength) || (packetLength < 0))
  {
    return;
  }

  // Restart CC1101 reception as soon as possible not to miss the next packet
  RestartReception();
  // Fill message structure
  message.len = packetLength;
  message.rssi = sx1276Driver.GetRssi();
  message.startTime_us = startTime_us;
  message.endTime_us = startTime_us + PREAMBLE_LENGTH_IN_US +
                       packetLength * BYTE_LENGTH_IN_US + GUARD_TIME_IN_US;
  message.action = MICRONET_ACTION_RF_NO_ACTION;
  messageFifo->PushIsr(message);

  // Only perform frequency tracking if the feature has been explicitly enabled
  if (freqTrackingNID != 0)
  {
    unsigned int networkId = message.data[MICRONET_NUID_OFFSET];
    networkId = (networkId << 8) | message.data[MICRONET_NUID_OFFSET + 1];
    networkId = (networkId << 8) | message.data[MICRONET_NUID_OFFSET + 2];
    networkId = (networkId << 8) | message.data[MICRONET_NUID_OFFSET + 3];

    // Only track if message is from the master of our network
    if ((message.data[MICRONET_MI_OFFSET] ==
         MICRONET_MESSAGE_ID_MASTER_REQUEST) &&
        (networkId == freqTrackingNID))
    {
      // cc1101Driver.UpdateFreqOffset();
    }
  }
}

void RfDriver::RestartReception()
{
  sx1276Driver.GoToIdle();
  sx1276Driver.SetFifoThreshold(13);
  sx1276Driver.SetPacketLength(SX1276_FIFO_MAX_SIZE);
  rfState = RF_STATE_RX_WAIT_SYNC;
  sx1276Driver.StartRx();
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
    RestartReception();
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
    RestartReception();

    ScheduleTransmit();
  }
  else if (rfState == RF_STATE_RX_WAIT_SYNC)
  {
    rfState = RF_STATE_TX_TRANSMIT;

    // Change CC1101 configuration for transmission
    sx1276Driver.GoToIdle();
    sx1276Driver.IrqOnTxFifoThreshold();
    sx1276Driver.SetFifoThreshold(8);

    // Fill FIFO with preamble's first byte
    sx1276Driver.WriteFifo(MICRONET_RF_PREAMBLE_BYTE);

    // Start transmission as soon as we have the first byte available in FIFO to
    // minimize latency
    sx1276Driver.StartTx();

    // Fill FIFO with rest of preamble and sync byte
    sx1276Driver.WriteFifo(static_cast<const uint8_t *>(preambleAndSync),
                           sizeof(preambleAndSync));

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
