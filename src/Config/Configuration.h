/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Configuration handler                                         *
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

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

/***************************************************************************/
/*                              Includes                                   */
/***************************************************************************/

#include "MicronetCodec.h"
#include "MicronetDevice.h"
#include <Arduino.h>
#include <stdint.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

/***************************************************************************/
/*                                Types                                    */
/***************************************************************************/

typedef enum
{
    RF_FREQ_SYSTEM_868 = 0,
    RF_FREQ_SYSTEM_915
} FreqSystem_t;

typedef enum
{
    SERIAL_TYPE_USB = 0,
    SERIAL_TYPE_BT,
    SERIAL_TYPE_WIFI
} SerialType_t;

typedef enum
{
    LINK_NMEA_EXT,
    LINK_NMEA_GNSS,
    LINK_MICRONET,
    LINK_COMPASS
} LinkId_t;

typedef enum
{
    COMPASS_HDG_VECTOR_X = 0,
    COMPASS_HDG_VECTOR_Y,
    COMPASS_HDG_VECTOR_Z,
    COMPASS_HDG_VECTOR_MX,
    COMPASS_HDG_VECTOR_MY,
    COMPASS_HDG_VECTOR_MZ
} CompassHdgVec_t;

typedef struct
{
    uint32_t        networkId;
    uint32_t        deviceId;
    float           waterSpeedFactor_per;
    float           waterTemperatureOffset_C;
    float           depthOffset_m;
    float           windSpeedFactor_per;
    float           windDirectionOffset_deg;
    float           headingOffset_deg;
    float           magneticVariation_deg;
    float           windShift;
    float           xMagOffset;
    float           yMagOffset;
    float           zMagOffset;
    FreqSystem_t    freqSystem;
    int8_t          timeZone_h;
    uint8_t         rmbWorkaround;
    uint8_t         windRepeater;
    uint8_t         spare;
    SerialType_t    nmeaLink;
    LinkId_t        gnssSource;
    LinkId_t        windSource;
    LinkId_t        depthSource;
    LinkId_t        speedSource;
    LinkId_t        compassSource;
    CompassHdgVec_t compassHdgVector;
} EEPROMConfig_t;

typedef struct
{
    bool    navCompassAvailable;
    bool    displayAvailable;
    Stream *nmeaLink;
} RAMConfig_t;

/***************************************************************************/
/*                               Classes                                   */
/***************************************************************************/

class Configuration
{
  public:
    Configuration();
    virtual ~Configuration();

    void Init();
    void LoadFromEeprom();
    void SaveToEeprom();
    void SaveCalibration(MicronetCodec &micronetCodec);
    void LoadCalibration(MicronetCodec *micronetCodec);
    void DeployConfiguration(MicronetDevice *micronetDevice);
    bool GetModifiedFlag();
    void SetModifiedFlag();

    // The following parameters are loaded/saved from/to EEPROM
    EEPROMConfig_t eeprom;
    // The following parameters are NOT loaded/saved from/to EEPROM
    RAMConfig_t ram;

  private:
    bool configModified;
};

/***************************************************************************/
/*                              Prototypes                                 */
/***************************************************************************/

#endif /* CONFIGURATION_H_ */
