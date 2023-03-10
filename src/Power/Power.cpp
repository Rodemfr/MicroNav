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

#ifndef CONFIG_PMU_SDA
#define CONFIG_PMU_SDA 21
#endif

#ifndef CONFIG_PMU_SCL
#define CONFIG_PMU_SCL 22
#endif

#ifndef CONFIG_PMU_IRQ
#define CONFIG_PMU_IRQ 35
#endif

#define COMMAND_EVENT_SHUTDOWN 0x00000001
#define COMMAND_EVENT_ALL      0x00000001

#define TASK_WAKEUP_PERIOD_MS 100 // Wake-up period of the processing task in milliseconds

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

Power::Power()
{
}

Power::~Power()
{
}

bool Power::Init()
{
    if (!AXPDriver.begin(Wire, AXP192_SLAVE_ADDRESS, PMU_I2C_SDA, PMU_I2C_SCL))
    {
        return false;
    }

    AXPDriver.setSysPowerDownVoltage(2700);

    AXPDriver.enableLDO2();        // RF
    AXPDriver.enableLDO3();        // GPS
    AXPDriver.disableDC2();        // Unused
    AXPDriver.enableExternalPin(); // ?
    AXPDriver.enableDC1();         // OLED

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

    AXPDriver.enableIRQ(XPOWERS_AXP192_BAT_INSERT_IRQ | XPOWERS_AXP192_BAT_REMOVE_IRQ |      // BATTERY
                        XPOWERS_AXP192_VBUS_INSERT_IRQ | XPOWERS_AXP192_VBUS_REMOVE_IRQ |    // VBUS
                        XPOWERS_AXP192_PKEY_SHORT_IRQ | XPOWERS_AXP192_PKEY_LONG_IRQ |       // POWER KEY
                        XPOWERS_AXP192_BAT_CHG_DONE_IRQ | XPOWERS_AXP192_BAT_CHG_START_IRQ | // CHARGE
                        // XPOWERS_AXP192_PKEY_NEGATIVE_IRQ | XPOWERS_AXP192_PKEY_POSITIVE_IRQ   |   //POWER KEY
                        XPOWERS_AXP192_TIMER_TIMEOUT_IRQ // Timer
    );

    UpdateStatus();

    powerEventGroup = xEventGroupCreate();
    xTaskCreate(PowerProcessingTask, "PowerTask", 16384, (void *)this, 6, &powerTaskHandle);

    return true;
}

void Power::Shutdown()
{
    xEventGroupSetBits(powerEventGroup, COMMAND_EVENT_SHUTDOWN);
}

PowerStatus_t &Power::GetStatus()
{
    return powerStatus;
}

void Power::PrintStatus()
{
    CONSOLE.print("Battery connected : ");
    CONSOLE.println(powerStatus.batteryConnected);
    CONSOLE.print("Battery charging : ");
    CONSOLE.println(powerStatus.batteryCharging);
    CONSOLE.print("Battery level : ");
    CONSOLE.println(powerStatus.batteryLevel_per);
    CONSOLE.print("Battery voltage : ");
    CONSOLE.println(powerStatus.batteryVoltage_V);
    CONSOLE.print("Battery current : ");
    CONSOLE.println(powerStatus.batteryCurrent_mA);
    CONSOLE.print("USB connected : ");
    CONSOLE.println(powerStatus.usbConnected);
    CONSOLE.print("USB voltage : ");
    CONSOLE.println(powerStatus.usbVoltage_V);
    CONSOLE.print("USB current : ");
    CONSOLE.println(powerStatus.usbCurrent_mA);
    CONSOLE.print("Temperature : ");
    CONSOLE.println(powerStatus.temperature_C);
}

/*
  Static entry point of the command processing task
  @param callingObject Pointer to the calling PanelManager instance
*/
void Power::PowerProcessingTask(void *callingObject)
{
    // Task entry points are static -> switch to non static processing method
    ((Power *)callingObject)->PowerProcessingLoop();
}

void Power::PowerProcessingLoop()
{
    uint32_t now;

    while (true)
    {
        // Wait for the next command
        EventBits_t commandFlags =
            xEventGroupWaitBits(powerEventGroup, COMMAND_EVENT_ALL, pdTRUE, pdFALSE, TASK_WAKEUP_PERIOD_MS / portTICK_PERIOD_MS);
        now = millis();

        if (commandFlags & COMMAND_EVENT_SHUTDOWN)
        {
            CommandShutdown();
        }

        UpdateStatus();
    }
}

void Power::UpdateStatus()
{
#define VOLTAGE_FILTERING_FACTOR     0.8f
#define CURRENT_FILTERING_FACTOR     0.95f
#define TEMPERATURE_FILTERING_FACTOR 0.9f
    powerStatus.batteryConnected = AXPDriver.isBatteryConnect();
    powerStatus.batteryCharging  = AXPDriver.isCharging();
    powerStatus.batteryLevel_per = AXPDriver.getBatteryPercent();
    powerStatus.batteryVoltage_V =
        (VOLTAGE_FILTERING_FACTOR * powerStatus.batteryVoltage_V) + ((1.0f - VOLTAGE_FILTERING_FACTOR) * AXPDriver.getBattVoltage() / 1000.0f);
    powerStatus.batteryCurrent_mA =
        (CURRENT_FILTERING_FACTOR * powerStatus.batteryCurrent_mA) + ((1.0f - CURRENT_FILTERING_FACTOR) * AXPDriver.getBattDischargeCurrent());
    powerStatus.usbConnected = AXPDriver.isVbusIn();
    powerStatus.usbVoltage_V =
        (VOLTAGE_FILTERING_FACTOR * powerStatus.usbVoltage_V) + ((1.0f - VOLTAGE_FILTERING_FACTOR) * AXPDriver.getVbusVoltage() / 1000.0f);
    powerStatus.usbCurrent_mA =
        (CURRENT_FILTERING_FACTOR * powerStatus.usbCurrent_mA) + ((1.0f - CURRENT_FILTERING_FACTOR) * AXPDriver.getVbusCurrent());
    powerStatus.temperature_C =
        (TEMPERATURE_FILTERING_FACTOR * powerStatus.temperature_C) + ((1.0f - TEMPERATURE_FILTERING_FACTOR) * AXPDriver.getTemperature());
}

void Power::CommandShutdown()
{
    AXPDriver.shutdown();
}
