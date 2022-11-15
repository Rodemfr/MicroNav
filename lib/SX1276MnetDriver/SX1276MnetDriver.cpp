/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Driver for SX1276                                             *
 * Author:   Ronan Demoment                                                *
 *           heavily based on RadioLib driver by Jan Gromes                *
 *           (https://github.com/jgromes/RadioLib)                         *
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

/***************************************************************************/
/*                              Includes                                   */
/***************************************************************************/

#include "SX1276MnetDriver.h"
#include "SX1276Regs.h"

#include <Arduino.h>
#include <RadioLib.h>
#include <SPI.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

#define SPI_WRITE_COMMAND 0x80

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
 * Constructor of SX1276MnetDriver
 * Initialize class attributes, SPI HW and pins
 */
SX1276MnetDriver::SX1276MnetDriver() {}

/*
 * Destructor of SX1276MnetDriver
 * Release SPI bus and reset pin config
 */
SX1276MnetDriver::~SX1276MnetDriver() {}

bool SX1276MnetDriver::Init(uint32_t sckPin, uint32_t mosiPin,
                            uint32_t miso_Pin, uint32_t csPin, uint32_t dio0Pin,
                            uint32_t dio1Pin, uint32_t rstPin) {
  this->sckPin = sckPin;
  this->mosiPin = mosiPin;
  this->miso_Pin = miso_Pin;
  this->csPin = csPin;
  this->dio0Pin = dio0Pin;
  this->dio1Pin = dio1Pin;
  this->rstPin = rstPin;

  pinMode(sckPin, OUTPUT);
  pinMode(mosiPin, OUTPUT);
  pinMode(miso_Pin, INPUT);
  pinMode(csPin, OUTPUT);
  pinMode(dio0Pin, INPUT);
  pinMode(dio1Pin, INPUT);

  // Pins startup state
  digitalWrite(csPin, HIGH);
  digitalWrite(sckPin, HIGH);
  digitalWrite(mosiPin, LOW);

  // Reset SX1276
  Reset();

  // Start SPI driver
  SPI.begin(sckPin, miso_Pin, mosiPin, csPin);
  // In  the context of MicronetToNMEA, only SX1276 is alone on its SPI bus, so
  // we can request SPI access here once for all here In case SX1276 would share
  // the SPI bus with other ICs, beginTransaction and endTransaction should be
  // moved into each SPI access member to avoid race conditions.
  SPI.beginTransaction(spiSettings);

  if (SpiReadRegister(RADIOLIB_SX127X_REG_VERSION) != SX1276_CHIP_VERSION) {
    return false;
  }

  return true;
}

uint8_t SX1276MnetDriver::SpiReadRegister(uint8_t addr) {
  uint8_t data;

  // Assert CS line
  digitalWrite(csPin, LOW);
  // Read register
  SPI.transfer(addr & 0x7f);
  data = SPI.transfer(0x00);
  // Release CS
  digitalWrite(csPin, LOW);

  return data;
}

void SX1276MnetDriver::SpiBurstReadRegister(uint8_t addr, uint8_t *data,
                                            uint16_t length) {
  // Assert CS line
  digitalWrite(csPin, LOW);
  // Read register
  SPI.transfer(addr & 0x7f);
  SPI.transferBytes(nullptr, data, length);
  // Release CS
  digitalWrite(csPin, LOW);
}

void SX1276MnetDriver::SpiWriteRegister(uint8_t addr, uint8_t value) {
  // Assert CS line
  digitalWrite(csPin, LOW);
  // Write register
  SPI.transfer(SPI_WRITE_COMMAND | addr);
  SPI.transfer(value);
  // Release CS
  digitalWrite(csPin, LOW);
}

void SX1276MnetDriver::SpiBurstWriteRegister(uint8_t addr, uint8_t *data,
                                             uint16_t length) {
  // Assert CS line
  digitalWrite(csPin, LOW);
  // Read register
  SPI.transfer(SPI_WRITE_COMMAND | addr);
  SPI.transferBytes(data, nullptr, length);
  // Release CS
  digitalWrite(csPin, LOW);
}

void SX1276MnetDriver::Reset() {
  pinMode(rstPin, OUTPUT);
  digitalWrite(rstPin, LOW);
  delay(1);r
  digitalWrite(rstPin, HIGH);
  delay(5);
}

void SX1276MnetDriver::SetBaseConfiguration(float br, float freqDev, float rxBw, uint16_t preambleLength) {
  // set mode to standby
  int16_t state = standby();

  // check currently active modem
  if(getActiveModem() != RADIOLIB_SX127X_FSK_OOK) {
    // set FSK mode
    setActiveModem(RADIOLIB_SX127X_FSK_OOK);
  }
  // enable/disable OOK
  setOOK(false);

  // set bit rate
  setBitRate(br);

  // set frequency deviation
  setFrequencyDeviation(freqDev);

  // set AFC bandwidth
  setAFCBandwidth(rxBw);

  // set AFC&AGC trigger to RSSI (both in OOK and FSK)
  setAFCAGCTrigger(RADIOLIB_SX127X_RX_TRIGGER_RSSI_INTERRUPT);

  // enable AFC
  setAFC(false);

  // set receiver bandwidth
  setRxBandwidth(rxBw);

  // set over current protection
  setCurrentLimit(60);

  // set preamble length
  setPreambleLength(preambleLength);

  // set default sync word
  uint8_t syncWord[] = {0x99};
  setSyncWord(syncWord, 1);

  // disable address filtering
  disableAddressFiltering();

  // set default RSSI measurement config
  setRSSIConfig(2);

  // set default encoding
  setEncoding(RADIOLIB_ENCODING_NRZ);

  // set default packet length mode
  variablePacketLengthMode();
}
