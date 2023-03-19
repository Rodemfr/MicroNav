/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Power Manager                                                 *
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

#ifndef POWER_H_
#define POWER_H_

/***************************************************************************/
/*                              Includes                                   */
/***************************************************************************/

// Using AXP192
#define XPOWERS_CHIP_AXP192

#include "XPowersLib.h"
#include <Arduino.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

/***************************************************************************/
/*                                Types                                    */
/***************************************************************************/

typedef struct
{
    bool  batteryConnected;
    bool  batteryCharging;
    float batteryLevel_per;
    float batteryVoltage_V;
    float batteryCurrent_mA;
    bool  usbConnected;
    float usbVoltage_V;
    float usbCurrent_mA;
    float temperature_C;
} PowerStatus_t;

typedef void (*ButtonCallback_t)(bool);

/***************************************************************************/
/*                               Classes                                   */
/***************************************************************************/

class Power
{
  public:
    Power();
    virtual ~Power();

    bool           Init();
    void           Shutdown();
    PowerStatus_t &GetStatus();
    void           RegisterButtonCallback(ButtonCallback_t callback);

  private:
    XPowersPMU         AXPDriver;
    PowerStatus_t      powerStatus;
    TaskHandle_t       powerTaskHandle;
    EventGroupHandle_t powerEventGroup;
    static Power      *objectPtr;
    ButtonCallback_t   buttonCallback;
    bool               firstBatteryQuery;

    static void StaticProcessingTask(void *callingObject);
    void        ProcessingTask();
    static void StaticIrqCallback();
    void        UpdateStatus();
    void        CommandShutdown();
    uint16_t    GetBatteryLevel(float voltage_V, float current_mA);
};

/***************************************************************************/
/*                              Prototypes                                 */
/***************************************************************************/

#endif /* POWER_H_ */
