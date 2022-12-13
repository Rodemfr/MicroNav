/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Board/Platform related configuration                          *
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

#ifndef BOARDCONFIG_H_
#define BOARDCONFIG_H_

/***************************************************************************/
/*                              Includes                                   */
/***************************************************************************/

#include "Globals.h"

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

// Selects on which I2C bus is connected compass as per Wiring library definition
#define NAVCOMPASS_I2C Wire

#define RF_CS0_PIN  18
#define RF_MOSI_PIN 27
#define RF_MISO_PIN 19
#define RF_SCK_PIN  5
#define RF_DIO0_PIN 26
#define RF_DIO1_PIN 33
#define RF_RST_PIN  23
#define RF_BUSY_PIN 32

// NMEA GNSS UART pins
#define GNSS_UBLOXM8N 1       // Set to one if your GNSS is a UBLOX M8N, 0 else. If set to one, GNSS will be automatically configured at startup
#define GNSS_SERIAL   Serial2
#define GNSS_BAUDRATE 9600
#define GNSS_RX_PIN   34
#define GNSS_TX_PIN   12

// The console to use for menu
#define CONSOLE          Serial
#define CONSOLE_BAUDRATE 115200

// Button for user interaction
#define BUTTON_PIN      38
#define BUTTON_PIN_MASK GPIO_SEL_38

// PMU connections
#define PMU_I2C_SDA 21
#define PMU_I2C_SCL 22
#define PMU_IRQ     35

/***************************************************************************/
/*                                Types                                    */
/***************************************************************************/

/***************************************************************************/
/*                              Prototypes                                 */
/***************************************************************************/

#endif /* BOARDCONFIG_H_ */
