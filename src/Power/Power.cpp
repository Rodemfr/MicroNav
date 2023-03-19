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

/***************************************************************************/
/*                              Includes                                   */
/***************************************************************************/

#include "Power.h"
#include "BoardConfig.h"
#include "Globals.h"

#include <Arduino.h>
#include <Wire.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

// Command masks for IRQ/Task communication
#define POWER_EVENT_SHUTDOWN 0x00000001
#define POWER_EVENT_IRQ      0x00000002
#define POWER_EVENT_ALL      0x00000003

// Wake-up period of the power processing task in milliseconds
#define TASK_WAKEUP_PERIOD_MS 100

// Those are the filtering factors for various battery indicators.
// Filtering them avoids current/voltage bursts to propagates
// noise to other values.
#define VOLTAGE_FILTERING_FACTOR       0.80f
#define CURRENT_FILTERING_FACTOR       0.80f
#define BATTERY_LEVEL_FILTERING_FACTOR 0.99f
#define TEMPERATURE_FILTERING_FACTOR   0.98f

// Estimated internal resistance of battery.
#define BATTERY_INTERNAL_RESISTANCE_OHM 0.150f

/***************************************************************************/
/*                             Local types                                 */
/***************************************************************************/

/***************************************************************************/
/*                           Local prototypes                              */
/***************************************************************************/

/***************************************************************************/
/*                               Globals                                   */
/***************************************************************************/

// Pointer to the class instance. Used by static IRQ callback to pass data to the object instance.
Power *Power::objectPtr;

// Voltage table used to estimate battery charge level
const int32_t Power::voltageTable[VOLTAGE_TABLE_ENTRIES] = {3000, 3650, 3700, 3740, 3760, 3795, 3840, 3910, 3980, 4070, 4150};

/***************************************************************************/
/*                              Functions                                  */
/***************************************************************************/

// Class constructor
Power::Power() : buttonCallback(nullptr), firstBatteryQuery(true)
{
}

// Class destructor
Power::~Power()
{
}

bool Power::Init()
{
    if (!AXPDriver.begin(Wire, AXP192_SLAVE_ADDRESS, PMU_I2C_SDA, PMU_I2C_SCL))
    {
        return false;
    }

    objectPtr         = this;
    firstBatteryQuery = true;

    AXPDriver.setSysPowerDownVoltage(2900);
    AXPDriver.setVbusVoltageLimit(XPOWERS_AXP192_VBUS_VOL_LIM_4V5);
    AXPDriver.setVbusCurrentLimit(XPOWERS_AXP192_VBUS_CUR_LIM_OFF);
    AXPDriver.setVbusCurrentLimit(XPOWERS_AXP192_VBUS_CUR_LIM_OFF);

    AXPDriver.enableLDO2();        // RF
    AXPDriver.enableLDO3();        // GPS
    AXPDriver.disableDC2();        // Unused
    AXPDriver.enableExternalPin(); // ?
    AXPDriver.enableDC1();         // OLED

    AXPDriver.setPowerKeyLongPressOnTime(XPOWERS_AXP192_LONGPRESS_1000MS);
    AXPDriver.setPowerKeyPressOffTime(XPOWERS_AXP192_POWEROFF_8S);
    AXPDriver.setPowerKeyPressOnTime(XPOWERS_POWERON_128MS);

    AXPDriver.disableTSPinMeasure();

    AXPDriver.enableTemperatureMeasure();

    AXPDriver.enableBattDetection();
    AXPDriver.enableVbusVoltageMeasure();
    AXPDriver.enableBattVoltageMeasure();
    AXPDriver.enableSystemVoltageMeasure();

    AXPDriver.setChargingLedMode(XPOWERS_CHG_LED_CTRL_CHG);

    AXPDriver.setChargerConstantCurr(XPOWERS_AXP192_CHG_CUR_280MA);
    AXPDriver.setChargerTerminationCurr(XPOWERS_AXP192_CHG_ITERM_LESS_10_PERCENT);
    AXPDriver.setChargeTargetVoltage(XPOWERS_AXP192_CHG_VOL_4V2);

    // Set the timing after one minute, the isWdtExpireIrq will be triggered in the loop interrupt function
    AXPDriver.setTimerout(1);

    UpdateStatus();

    powerEventGroup = xEventGroupCreate();
    xTaskCreate(StaticProcessingTask, "PowerTask", 16384, (void *)this, 6, &powerTaskHandle);

    AXPDriver.disableIRQ(XPOWERS_AXP192_ALL_IRQ);
    AXPDriver.clearIrqStatus();
    AXPDriver.enableIRQ(XPOWERS_AXP192_PKEY_SHORT_IRQ | XPOWERS_AXP192_PKEY_LONG_IRQ);
    pinMode(PMU_IRQ, INPUT);
    attachInterrupt(PMU_IRQ, StaticIrqCallback, FALLING);

    return true;
}

void Power::RegisterButtonCallback(ButtonCallback_t callback)
{
    buttonCallback = callback;
}

void Power::Shutdown()
{
    xEventGroupSetBits(powerEventGroup, POWER_EVENT_SHUTDOWN);
}

PowerStatus_t &Power::GetStatus()
{
    return powerStatus;
}

/*
  Static entry point of the command processing task
  @param callingObject Pointer to the calling PanelManager instance
*/
void Power::StaticProcessingTask(void *callingObject)
{
    // Task entry points are static -> switch to non static processing method
    ((Power *)callingObject)->ProcessingTask();
}

void Power::ProcessingTask()
{
    while (true)
    {
        // Wait for the next command
        EventBits_t commandFlags = xEventGroupWaitBits(powerEventGroup, POWER_EVENT_ALL, pdTRUE, pdFALSE, TASK_WAKEUP_PERIOD_MS / portTICK_PERIOD_MS);

        if (commandFlags & POWER_EVENT_SHUTDOWN)
        {
            CommandShutdown();
        }
        if (commandFlags & POWER_EVENT_IRQ)
        {
            AXPDriver.getIrqStatus();

            if (buttonCallback != nullptr)
            {
                if (AXPDriver.isPekeyShortPressIrq())
                {
                    buttonCallback(false);
                }
                if (AXPDriver.isPekeyLongPressIrq())
                {
                    buttonCallback(true);
                }
            }

            AXPDriver.clearIrqStatus();
        }

        UpdateStatus();
    }
}

void IRAM_ATTR Power::StaticIrqCallback()
{
    BaseType_t scheduleChange = pdFALSE;

    xEventGroupSetBitsFromISR(objectPtr->powerEventGroup, POWER_EVENT_IRQ, &scheduleChange);
    portYIELD_FROM_ISR(scheduleChange);
}

void Power::UpdateStatus()
{
    float batVoltage_V  = AXPDriver.getBattVoltage() / 1000.0f;
    float batCurrent_mA = AXPDriver.getBatteryChargeCurrent() - AXPDriver.getBattDischargeCurrent();

    powerStatus.batteryConnected = AXPDriver.isBatteryConnect();
    powerStatus.batteryCharging  = AXPDriver.isCharging();
    powerStatus.usbConnected     = AXPDriver.isVbusIn();

    if (firstBatteryQuery)
    {
        firstBatteryQuery             = false;
        powerStatus.batteryVoltage_V  = batVoltage_V;
        powerStatus.batteryCurrent_mA = batCurrent_mA;
        powerStatus.batteryLevel_per  = GetBatteryLevel(batVoltage_V, batCurrent_mA);
        powerStatus.usbVoltage_V      = AXPDriver.getVbusVoltage();
        powerStatus.usbCurrent_mA     = AXPDriver.getVbusCurrent();
        powerStatus.temperature_C     = AXPDriver.getTemperature();
    }
    else
    {
        powerStatus.batteryVoltage_V = (VOLTAGE_FILTERING_FACTOR * powerStatus.batteryVoltage_V) + ((1.0f - VOLTAGE_FILTERING_FACTOR) * batVoltage_V);
        powerStatus.batteryCurrent_mA =
            (CURRENT_FILTERING_FACTOR * powerStatus.batteryCurrent_mA) + ((1.0f - CURRENT_FILTERING_FACTOR) * batCurrent_mA);
        powerStatus.usbVoltage_V =
            (VOLTAGE_FILTERING_FACTOR * powerStatus.usbVoltage_V) + ((1.0f - VOLTAGE_FILTERING_FACTOR) * AXPDriver.getVbusVoltage() / 1000.0f);
        powerStatus.usbCurrent_mA =
            (CURRENT_FILTERING_FACTOR * powerStatus.usbCurrent_mA) + ((1.0f - CURRENT_FILTERING_FACTOR) * AXPDriver.getVbusCurrent());
        powerStatus.temperature_C =
            (TEMPERATURE_FILTERING_FACTOR * powerStatus.temperature_C) + ((1.0f - TEMPERATURE_FILTERING_FACTOR) * AXPDriver.getTemperature());
        powerStatus.batteryLevel_per =
            BATTERY_LEVEL_FILTERING_FACTOR * powerStatus.batteryLevel_per +
            (1.0f - BATTERY_LEVEL_FILTERING_FACTOR) * GetBatteryLevel(powerStatus.batteryVoltage_V, powerStatus.batteryCurrent_mA);
    }
}

void Power::CommandShutdown()
{
    AXPDriver.shutdown();
}

uint16_t Power::GetBatteryLevel(float voltage_V, float current_mA)
{
    if (!AXPDriver.isBatteryConnect())
    {
        return -1;
    }
    
    int32_t correctedVoltage_mV = voltage_V * 1000.0f - BATTERY_INTERNAL_RESISTANCE_OHM * current_mA;

    if (correctedVoltage_mV < voltageTable[0])
        return 0;
    
    for (int i = 1; i < VOLTAGE_TABLE_ENTRIES; i++)
    {
        if (correctedVoltage_mV < voltageTable[i])
            return i * (100 / (VOLTAGE_TABLE_ENTRIES - 1)) - ((VOLTAGE_TABLE_ENTRIES - 1) * (voltageTable[i] - correctedVoltage_mV)) / (voltageTable[i] - voltageTable[i - 1]);
    }

    return 100;
}
