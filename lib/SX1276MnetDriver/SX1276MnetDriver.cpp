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
#include "BoardConfig.h"
#include "SX1276Regs.h"

#include <Arduino.h>
#include <RadioLib.h>
#include <SPI.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

#define SPI_READ_COMMAND 0b00000000
#define SPI_WRITE_COMMAND 0b10000000

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

bool SX1276MnetDriver::Init(void) {
  pinMode(RF_SCK_PIN, OUTPUT);
  pinMode(RF_MOSI_PIN, OUTPUT);
  pinMode(RF_MISO_PIN, INPUT);
  pinMode(RF_CS0_PIN, OUTPUT);
  pinMode(RF_DIO0_PIN, INPUT);

  // Pins startup state
  digitalWrite(RF_CS0_PIN, HIGH);
  digitalWrite(RF_SCK_PIN, HIGH);
  digitalWrite(RF_MOSI_PIN, LOW);

  // Start SPI driver
  SPI.begin(RF_SCK_PIN, RF_MISO_PIN, RF_MOSI_PIN, RF_CS0_PIN);
  // In  the context of MicronetToNMEA, only CC1101 driver is using SPI, so we
  // can transaction once for all here and end the transaction in the destructor
  // In case CC1101 would share the SPI bus with other ICs, beginTransaction
  // should be moved into each SPI access member to avoid race conditions.
  SPI.beginTransaction(spiSettings);

  SPIgetRegValue(RADIOLIB_SX127X_REG_VERSION);

  return true;
}
void SX1276MnetDriver::SPItransfer(uint8_t cmd, uint8_t reg, uint8_t *dataOut,
                                   uint8_t *dataIn, uint8_t numBytes) {
  SPI.beginTransaction(spiSettings);
  digitalWrite(RF_CS0_PIN, LOW);

  // send SPI register address with access command
  SPI.transfer(reg | cmd);

  // send data or get response
  if (cmd == SPI_WRITE_COMMAND) {
    if (dataOut != NULL) {
      for (size_t n = 0; n < numBytes; n++) {
        SPI.transfer(dataOut[n]);
      }
    }
  } else if (cmd == SPI_READ_COMMAND) {
    if (dataIn != NULL) {
      for (size_t n = 0; n < numBytes; n++) {
        dataIn[n] = SPI.transfer(0x00);
      }
    }
  }

  // release CS
  digitalWrite(RF_CS0_PIN, LOW);

  // end SPI transaction
  SPI.endTransaction();
}

uint8_t SX1276MnetDriver::SPIreadRegister(uint8_t reg) {
  uint8_t resp = 0;
  SPItransfer(SPI_READ_COMMAND, reg, NULL, &resp, 1);
  return (resp);
}

int16_t SX1276MnetDriver::SPIgetRegValue(uint8_t reg, uint8_t msb,
                                         uint8_t lsb) {
  if ((msb > 7) || (lsb > 7) || (lsb > msb)) {
    return (RADIOLIB_ERR_INVALID_BIT_RANGE);
  }

  uint8_t rawValue = SPIreadRegister(reg);
  uint8_t maskedValue =
      rawValue & ((0b11111111 << lsb) & (0b11111111 >> (7 - msb)));
  return (maskedValue);
}
