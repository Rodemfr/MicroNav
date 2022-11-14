/***************************************************************************
 *                                                                         *
 * Project:  MicronetToNMEA                                                *
 * Purpose:  Decode data from Micronet devices send it on an NMEA network  *
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

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

// Select EU (868MHz) or not-EU (915MHz) Micronet frequency
// 0 -> EU (868Mhz)
// 1 -> non-EU (915Mhz)
#define FREQUENCY_SYSTEM 0

// Selects on which I2C bus is connected compass as per Wiring library definition
#define NAVCOMPASS_I2C Wire1
#define NAVCOMPASS_I2C_SDA 32
#define NAVCOMPASS_I2C_SCL 33

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

// USB UART params
#define USB_NMEA     Serial
#define USB_BAUDRATE 115200

// Wired UART params
#define WIRED_NMEA     Serial1
#define WIRED_BAUDRATE 115200
#define WIRED_RX_PIN   13
#define WIRED_TX_PIN   14

// The console to use for menu and NMEA output
#define CONSOLE  USB_NMEA
#define NMEA_EXT WIRED_NMEA

// Button for user interaction
#define BUTTON_PIN      38
#define BUTTON_PIN_MASK GPIO_SEL_38

// PMU connections
#define PMU_I2C_SDA 21
#define PMU_I2C_SCL 22
#define PMU_IRQ     35

// Defines with data comes from which link
// LINK_NMEA_EXT -> data comes from external NMEA link (WIRED_NMEA)
// LINK_NMEA_GNSS -> data comes from GNSS NMEA link (GNSS_SERIAL)
// LINK_MICRONET -> data comes from Micronet network
// LINK_COMPASS -> data comes from LSM303 (NAVCOMPASS_I2C)
#define NAV_SOURCE_LINK     LINK_NMEA_EXT  // Navigation data (RMB)
#define GNSS_SOURCE_LINK    LINK_NMEA_EXT // Positionning data (RMC, GGA, VTG)
#define WIND_SOURCE_LINK    LINK_NMEA_EXT  // Wind data (MWV)
#define DEPTH_SOURCE_LINK   LINK_NMEA_EXT  // Depth data (DPT)
#define SPEED_SOURCE_LINK   LINK_NMEA_EXT  // Speed data (SPD, LOG)
#define VOLTAGE_SOURCE_LINK LINK_NMEA_EXT  // Battery voltage data (XDG)
#define SEATEMP_SOURCE_LINK LINK_NMEA_EXT  // Temperature data (STP)
#define COMPASS_SOURCE_LINK LINK_NMEA_EXT   // Heading data (HDG)

// Navigation softwares can send a wrong RMB sentence and invert "FROM" and "TO" fields
// If you see your Micronet display showing the "FROM" waypoint name instead of the "TO"
// on the DTW & BTW pages, then change the following configuration key to 1
#define INVERTED_RMB_WORKAROUND 1

// In case your displays would have difficulties to receive data from the Tacktick wind
// transducer because of a poor signal/noise ratio, you can ask MicronetToNMEA to repeat
// the values of AWA & AWS on the network by setting MICRONET_WIND_REPEATER to 1. Set it
// to 0 else.
#define MICRONET_WIND_REPEATER 1

/***************************************************************************/
/*                                Types                                    */
/***************************************************************************/

/***************************************************************************/
/*                              Prototypes                                 */
/***************************************************************************/

#endif /* BOARDCONFIG_H_ */
