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

#ifndef M8NDRIVER_H_
#define M8NDRIVER_H_

#include <Arduino.h>

#define M8N_GGA_ENABLE  0x00000001
#define M8N_VTG_ENABLE  0x00000002
#define M8N_RMC_ENABLE  0x00000004
#define M8N_HISPEED_NAV 0x00000008

class M8NDriver
{
  public:
    M8NDriver();
    virtual ~M8NDriver();

    void Start(uint32_t config);
    void Sleep();

  private:
    static const PROGMEM uint8_t ClearConfig[];
    static const PROGMEM uint8_t UART1_38400[];
    static const PROGMEM uint8_t Navrate5hz[];
    static const PROGMEM uint8_t GNSSSetup[];
    static const PROGMEM char    M8NDriver::SleepMode[];

    void GPS_SendConfig(const uint8_t *progmemPtr, uint8_t arraySize);
    void GPS_SendPUBX(const char pubxMsg[]);
};

#endif /* M8NDRIVER_H_ */
