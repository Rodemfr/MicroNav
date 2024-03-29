/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Driver for UBlox GNSS                                         *
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

#include "UbloxDriver.h"
#include "BoardConfig.h"

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

const PROGMEM uint8_t UbloxDriver::ClearConfig[] = {0xB5, 0x62, 0x06, 0x09, 0x0D, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x01, 0x19, 0x98};

const PROGMEM uint8_t UbloxDriver::UART1_38400[] = {0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00,
                                                  0x00, 0x96, 0x00, 0x00, 0x07, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x92, 0x8A};

const PROGMEM uint8_t UbloxDriver::GNSSSetup[]  = {0xB5, 0x62, 0x06, 0x3E, 0x3C, 0x00, 0x00, 0x00, 0x20, 0x07, 0x00, 0x08, 0x10, 0x00, 0x01, 0x00, 0x01,
                                                0x01, 0x01, 0x01, 0x03, 0x00, 0x01, 0x00, 0x01, 0x01, 0x02, 0x04, 0x08, 0x00, 0x01, 0x00, 0x01, 0x01,
                                                0x03, 0x08, 0x10, 0x00, 0x00, 0x00, 0x01, 0x01, 0x04, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x01, 0x05,
                                                0x00, 0x03, 0x00, 0x01, 0x00, 0x01, 0x01, 0x06, 0x08, 0x0E, 0x00, 0x01, 0x00, 0x01, 0x01, 0x30, 0xAD};

const PROGMEM char UbloxDriver::SleepMode[] = {0xB5, 0x62, 0x02, 0x41, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                             0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x61, 0x6B};

// PUBX Messages
const PROGMEM char DTM_off[] = "$PUBX,40,DTM,0,0,0,0,0,0*46";
const PROGMEM char GBS_off[] = "$PUBX,40,GBS,0,0,0,0,0,0*4D";
const PROGMEM char GGA_on[]  = "$PUBX,40,GGA,0,1,0,0,0,0*5B"; // GGA on
const PROGMEM char GGA_off[] = "$PUBX,40,GGA,0,0,0,0,0,0*5A"; // GGA off
const PROGMEM char GLL_off[] = "$PUBX,40,GLL,0,0,0,0,0,0*5C";
const PROGMEM char GNS_off[] = "$PUBX,40,GNS,0,0,0,0,0,0*41";
const PROGMEM char GRS_off[] = "$PUBX,40,GRS,0,0,0,0,0,0*5D";
const PROGMEM char GSA_off[] = "$PUBX,40,GSA,0,0,0,0,0,0*4E";
const PROGMEM char GST_off[] = "$PUBX,40,GST,0,0,0,0,0,0*5B";
const PROGMEM char GSV_off[] = "$PUBX,40,GSV,0,0,0,0,0,0*59";
const PROGMEM char RMC_on[]  = "$PUBX,40,RMC,0,1,0,0,0,0*46"; // RMC on
const PROGMEM char RMC_off[] = "$PUBX,40,RMC,0,0,0,0,0,0*47"; // RMC off
const PROGMEM char VLW_off[] = "$PUBX,40,VLW,0,0,0,0,0,0*56";
const PROGMEM char VTG_on[]  = "$PUBX,40,VTG,0,1,0,0,0,0*5F"; // VTG on
const PROGMEM char VTG_off[] = "$PUBX,40,VTG,0,0,0,0,0,0*5E"; // VTG off
const PROGMEM char THS_off[] = "$PUBX,40,THS,0,0,0,0,0,0*54";
const PROGMEM char ZDA_off[] = "$PUBX,40,ZDA,0,0,0,0,0,0*44";

#define GNSS_SERIAL_BUFFER_SIZE 512

/***************************************************************************/
/*                             Local types                                 */
/***************************************************************************/

/***************************************************************************/
/*                           Local prototypes                              */
/***************************************************************************/

/***************************************************************************/
/*                               Globals                                   */
/***************************************************************************/

uint8_t gnss_serial_rx_buffer[GNSS_SERIAL_BUFFER_SIZE];

/***************************************************************************/
/*                              Functions                                  */
/***************************************************************************/

UbloxDriver::UbloxDriver()
{
}

UbloxDriver::~UbloxDriver()
{
}

void UbloxDriver::GPS_SendConfig(const uint8_t *progmemPtr, uint8_t arraySize)
{
    uint8_t byteread, index;
    for (index = 0; index < arraySize; index++)
    {
        byteread = pgm_read_byte_near(progmemPtr++);
        if (byteread < 0x10)
        {
        }
    }
    progmemPtr = progmemPtr - arraySize;

    for (index = 0; index < arraySize; index++)
    {
        byteread = pgm_read_byte_near(progmemPtr++);
        GNSS_SERIAL.write(byteread);
    }
    delay(100);
}

void UbloxDriver::GPS_SendPUBX(const char pubxMsg[])
{
    GNSS_SERIAL.println(pubxMsg);
}

void UbloxDriver::Start(uint32_t config)
{
    GPS_SendConfig(ClearConfig, 21);
    delay(500);
    GPS_SendConfig(UART1_38400, 28);
    GNSS_SERIAL.begin(38400);
    delay(100);
    GNSS_SERIAL.println("");
    GPS_SendConfig(GNSSSetup, 68);
    delay(200);
    GNSS_SERIAL.println("");
    GPS_SendPUBX(DTM_off);
    GPS_SendPUBX(GBS_off);
    if (config & NMEA_GGA_ENABLE)
        GPS_SendPUBX(GGA_on);
    else
        GPS_SendPUBX(GGA_off);

    GPS_SendPUBX(GLL_off);
    GPS_SendPUBX(GNS_off);
    GPS_SendPUBX(GRS_off);
    GPS_SendPUBX(GSA_off);
    GPS_SendPUBX(GST_off);
    GPS_SendPUBX(GSV_off);
    if (config & NMEA_RMC_ENABLE)
        GPS_SendPUBX(RMC_on);
    else
        GPS_SendPUBX(RMC_off);

    GPS_SendPUBX(VLW_off);
    if (config & NMEA_VTG_ENABLE)
        GPS_SendPUBX(VTG_on);
    else
        GPS_SendPUBX(VTG_off);
    // GPS_SendPUBX(THS_off);
    GPS_SendPUBX(ZDA_off);
}

void UbloxDriver::Sleep(void)
{
    // first send dumb data to make sure its on
    GNSS_SERIAL.write(0xFF);
    GNSS_SERIAL.write(SleepMode, sizeof(SleepMode));
}
