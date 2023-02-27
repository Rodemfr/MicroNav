/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  NMEA/Micronet bridge                                          *
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

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#include "BoardConfig.h"
#include "Configuration.h"
#include "Globals.h"
#include "MenuManager.h"
#include "Micronet.h"
#include "MicronetCodec.h"
#include "MicronetMessageFifo.h"
#include "NavCompass.h"
#include "PanelManager.h"
#include "Version.h"

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

#define MAX_SCANNED_NETWORKS 5

/***************************************************************************/
/*                             Local types                                 */
/***************************************************************************/

/***************************************************************************/
/*                           Local prototypes                              */
/***************************************************************************/

void PrintByte(uint8_t data);
void PrintInt(uint32_t data);
void PrintRawMessage(MicronetMessage_t *message, uint32_t lastMasterRequest_us);
void PrintNetworkMap(NetworkMap_t *networkMap);

void MenuAbout();
void MenuConvertToNmea();
void MenuCalibrateMagnetoMeter();
void MenuDebug1();
void MenuDebug2();

/***************************************************************************/
/*                               Globals                                   */
/***************************************************************************/

bool firstLoop;

MenuEntry_t mainMenu[] = {
    {"MicroNav", nullptr}, {"Start NMEA conversion", MenuConvertToNmea}, {"Debug 1", MenuDebug1}, {"Debug 2", MenuDebug2}, {nullptr, nullptr}};

/***************************************************************************/
/*                              Functions                                  */
/***************************************************************************/

void setup()
{
    // Configure power supply
    Wire.begin(PMU_I2C_SDA, PMU_I2C_SCL);
    gPower.Init();

    // Load configuration from EEPROM
    gConfiguration.Init();
    gConfiguration.LoadFromEeprom();

    // Init USB serial link
    CONSOLE.begin(CONSOLE_BAUDRATE);

    CONSOLE.print("MicroNav v");
    CONSOLE.print(SW_MAJOR_VERSION);
    CONSOLE.print(".");
    CONSOLE.print(SW_MINOR_VERSION);
    CONSOLE.print(".");
    CONSOLE.println(SW_PATCH_VERSION);

    // Init GNSS NMEA serial link
    GNSS_SERIAL.begin(GNSS_BAUDRATE, SERIAL_8N1, GNSS_RX_PIN, GNSS_TX_PIN);

    // Let time for serial drivers to set-up
    delay(250);

#if (GNSS_UBLOXM8N == 1)
    CONSOLE.println("Configuring UBlox GNSS");
    gM8nDriver.Start(NMEA_GGA_ENABLE | NMEA_VTG_ENABLE | NMEA_RMC_ENABLE);
#endif

    // Setup main menu
    gMenuManager.SetMenu(mainMenu);

    CONSOLE.print("Initializing SX1276 ... ");
    // Check connection to SX1276
    if (!gRfDriver.Init(&gRxMessageFifo))
    {
        while (1)
        {
            CONSOLE.println("Failed");
            CONSOLE.println("System Halted");
            delay(1000);
        }
    }
    CONSOLE.println("OK");

    CONSOLE.print("Initializing navigation compass ... ");
    if (!gNavCompass.Init())
    {
        CONSOLE.println("NOT DETECTED");
        gConfiguration.ram.navCompassAvailable = false;
    }
    else
    {
        CONSOLE.print(gNavCompass.GetDeviceName().c_str());
        CONSOLE.println(" Found");
        gConfiguration.ram.navCompassAvailable = true;
    }

    CONSOLE.print("Initializing display ... ");
    if (!gPanelDriver.Init())
    {
        CONSOLE.println("NOT DETECTED");
        gConfiguration.ram.displayAvailable = false;
    }
    else
    {
        CONSOLE.println(" Found");
        gConfiguration.ram.displayAvailable = true;
    }

    gMicronetCodec.SetSwVersion(SW_MAJOR_VERSION, SW_MINOR_VERSION);
    gPanelDriver.SetNavigationData(&gMicronetCodec.navData);

    // Start listening
    gRfDriver.Start();

    // Display serial menu
    gMenuManager.PrintMenu();

    // For the main loop to know when it is executing for the first time
    firstLoop = true;
}

void loop()
{
    // If this is the first loop, we verify if we are already attached to a
    // Micronet network. if yes, We directly jump to NMEA conversion mode.
    if (firstLoop)
    {
        MenuConvertToNmea();
        gMenuManager.PrintMenu();
    }

    // Process console input
    while (CONSOLE.available() > 0)
    {
        gMenuManager.PushChar(CONSOLE.read());
    }

    firstLoop = false;
}

void PrintByte(uint8_t data)
{
    if (data < 16)
    {
        CONSOLE.print("0");
    }
    CONSOLE.print(data, HEX);
}

void PrintInt(uint32_t data)
{
    PrintByte((data >> 24) & 0x0ff);
    PrintByte((data >> 16) & 0x0ff);
    PrintByte((data >> 8) & 0x0ff);
    PrintByte(data & 0x0ff);
}

void PrintRawMessage(MicronetMessage_t *message, uint32_t lastMasterRequest_us)
{
    if (message->len < MICRONET_PAYLOAD_OFFSET)
    {
        CONSOLE.print("Invalid message (");
        CONSOLE.print((int)message->rssi);
        CONSOLE.print(", ");
        CONSOLE.print((int)(message->startTime_us - lastMasterRequest_us));
        CONSOLE.println(")");
    }

    for (int j = 0; j < 4; j++)
    {
        PrintByte(message->data[j]);
    }
    CONSOLE.print(" ");

    for (int j = 4; j < 8; j++)
    {
        PrintByte(message->data[j]);
    }
    CONSOLE.print(" ");

    for (int j = 8; j < 14; j++)
    {
        PrintByte(message->data[j]);
        CONSOLE.print(" ");
    }

    CONSOLE.print(" -- ");

    for (int j = 14; j < message->len; j++)
    {
        PrintByte(message->data[j]);
        CONSOLE.print(" ");
    }

    CONSOLE.print(" (");
    CONSOLE.print((int)message->rssi);
    CONSOLE.print(", ");
    CONSOLE.print((int)(message->startTime_us - lastMasterRequest_us));
    CONSOLE.print(")");

    CONSOLE.println();
}

void PrintNetworkMap(NetworkMap_t *networkMap)
{
    CONSOLE.print("Network ID : 0x");
    PrintInt(networkMap->networkId);
    CONSOLE.println("");

    CONSOLE.print("Nb Devices : ");
    CONSOLE.println(networkMap->nbSyncSlots);
    CONSOLE.print("Master : ");
    CONSOLE.print(" : 0x");
    PrintInt(networkMap->masterDevice);
    CONSOLE.println("");
    CONSOLE.print("First slot : ");
    CONSOLE.print(networkMap->firstSlot);
    CONSOLE.println("");

    for (uint32_t i = 0; i < networkMap->nbSyncSlots; i++)
    {
        CONSOLE.print("S");
        CONSOLE.print(i);
        CONSOLE.print(" : 0x");
        PrintInt(networkMap->syncSlot[i].deviceId);
        CONSOLE.print(" ");
        if (networkMap->syncSlot[i].start_us > 0)
        {
            CONSOLE.print(networkMap->syncSlot[i].payloadBytes);
            CONSOLE.print(" ");
            CONSOLE.print(networkMap->syncSlot[i].start_us - networkMap->firstSlot);
            CONSOLE.print(" ");
            CONSOLE.println(networkMap->syncSlot[i].length_us);
        }
        else
        {
            CONSOLE.println("-");
        }
    }

    CONSOLE.print("Async : ");
    CONSOLE.print(" ");
    CONSOLE.print(networkMap->asyncSlot.payloadBytes);
    CONSOLE.print(" ");
    CONSOLE.print(networkMap->asyncSlot.start_us - networkMap->firstSlot);
    CONSOLE.print(" ");
    CONSOLE.println(networkMap->asyncSlot.length_us);

    for (uint32_t i = 0; i < networkMap->nbAckSlots; i++)
    {
        CONSOLE.print("A");
        CONSOLE.print(i);
        CONSOLE.print(" : 0x");
        PrintInt(networkMap->ackSlot[i].deviceId);
        CONSOLE.print(" ");
        CONSOLE.print(networkMap->ackSlot[i].payloadBytes);
        CONSOLE.print(" ");
        CONSOLE.print(networkMap->ackSlot[i].start_us - networkMap->firstSlot);
        CONSOLE.print(" ");
        CONSOLE.println(networkMap->ackSlot[i].length_us);
    }
    CONSOLE.println("");
}

void MenuConvertToNmea()
{
    bool                exitNmeaLoop = false;
    MicronetMessage_t  *rxMessage;
    MicronetMessageFifo txMessageFifo;
    uint32_t            lastHeadingTime = millis();

    CONSOLE.println("Starting MicroNav...");

    // Load sensor calibration data into Micronet codec
    gConfiguration.LoadCalibration(&gMicronetCodec);

    // Configure Micronet device according to board configuration
    gConfiguration.DeployConfiguration(&gMicronetDevice);

    // Enable frequency tracking to compensate for XTAL drift
    gRfDriver.EnableFrequencyTracking(gConfiguration.eeprom.networkId);

    gRxMessageFifo.ResetFifo();

    do
    {
        if ((rxMessage = gRxMessageFifo.Peek()) != nullptr)
        {
            gMicronetDevice.ProcessMessage(rxMessage, &txMessageFifo);
            gRfDriver.Transmit(&txMessageFifo);

            gDataBridge.UpdateMicronetData();

            if ((gMicronetCodec.navData.calibrationUpdated) || (gConfiguration.GetModifiedFlag()))
            {
                gMicronetCodec.navData.calibrationUpdated = false;
                gConfiguration.SaveCalibration(gMicronetCodec);
                gConfiguration.DeployConfiguration(&gMicronetDevice);
            }

            gRxMessageFifo.DeleteMessage();
        }

        while (GNSS_SERIAL.available() > 0)
        {
            gDataBridge.PushNmeaChar(GNSS_SERIAL.read(), LINK_NMEA_GNSS);
        }

        char c;
        while (gConfiguration.ram.nmeaLink->available() > 0)
        {
            c = gConfiguration.ram.nmeaLink->read();
            if (((void *)(&CONSOLE) == (void *)(gConfiguration.ram.nmeaLink)) && (c == 0x1b))
            {
                CONSOLE.println("ESC key pressed, stopping conversion.");
                exitNmeaLoop = true;
            }
            gDataBridge.PushNmeaChar(c, LINK_NMEA_EXT);
        }

        // Only execute magnetic heading code if navigation compass is available
        if (gConfiguration.ram.navCompassAvailable == true)
        {
            // Handle magnetic compass
            // Only request new reading if previous is at least 100ms old
            if ((millis() - lastHeadingTime) > 100)
            {
                lastHeadingTime = millis();
                gDataBridge.UpdateCompassData(gNavCompass.GetHeading() + gMicronetCodec.navData.headingOffset_deg);
            }
        }

        if ((void *)(&CONSOLE) != (void *)(gConfiguration.ram.nmeaLink))
        {
            while (CONSOLE.available() > 0)
            {
                if (CONSOLE.read() == 0x1b)
                {
                    CONSOLE.println("ESC key pressed, stopping conversion.");
                    exitNmeaLoop = true;
                }
            }
        }

        gMicronetCodec.navData.UpdateValidity();

        gMicronetDevice.Yield();
        gPanelDriver.SetNetworkStatus(gMicronetDevice.GetDeviceInfo());
        yield();

    } while (!exitNmeaLoop);

    gRfDriver.DisableFrequencyTracking();
}

void MenuDebug1()
{
    CONSOLE.println("Off");
    gPanelDriver.LowPower(true);
}

void MenuDebug2()
{
    CONSOLE.println("On");
    gPanelDriver.LowPower(false);
}
