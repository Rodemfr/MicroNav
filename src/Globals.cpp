/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Global variables used by SW                                   *
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

#include "Globals.h"

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

/***************************************************************************/
/*                                Macros                                   */
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

RfDriver            gRfDriver;                        // RF Driver object
MenuManager         gMenuManager;                     // Menu manager object
MicronetCodec       gMicronetCodec;                   // Codec used by MicronetDevice
MicronetMessageFifo gRxMessageFifo;                   // Micronet message fifo store
Configuration       gConfiguration;                   // Global configuration
NavCompass          gNavCompass;                      // Navigation compass
UbloxDriver         gM8nDriver;                       // GNSS Driver
PanelManager        gPanelDriver;                     // Display driver
BluetoothSerial     gBtSerial;                        // Bluetooth driver
NmeaBridge          gDataBridge(&gMicronetCodec);     // NMEA Bridge
MicronetDevice      gMicronetDevice(&gMicronetCodec); // Micronet Device
Power               gPower;                           // Power Manager

/***************************************************************************/
/*                              Functions                                  */
/***************************************************************************/
