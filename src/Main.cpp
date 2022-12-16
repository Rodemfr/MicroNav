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
#include <axp20x.h>

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
void PrintNetworkMap(MicronetCodec::NetworkMap *networkMap);
void PrintMessageFifo(MicronetMessageFifo &messageFifo);

void MenuAbout();
void MenuScanNetworks();
void MenuAttachNetwork();
void MenuConvertToNmea();
void MenuScanTraffic();
void MenuCalibrateMagnetoMeter();
void MenuTestRfQuality();

/***************************************************************************/
/*                               Globals                                   */
/***************************************************************************/

AXP20X_Class pmu;

bool firstLoop;

MenuEntry_t mainMenu[] = {{"MicroNav", nullptr},
                          {"General info on MicroNav", MenuAbout},
                          {"Scan Micronet networks", MenuScanNetworks},
                          {"Attach converter to a network", MenuAttachNetwork},
                          {"Start NMEA conversion", MenuConvertToNmea},
                          {"Scan surrounding Micronet traffic", MenuScanTraffic},
                          {"Calibrate magnetometer", MenuCalibrateMagnetoMeter},
                          {"Test RF quality", MenuTestRfQuality},
                          {nullptr, nullptr}};

/***************************************************************************/
/*                              Functions                                  */
/***************************************************************************/

void setup()
{
    // Configure power supply
    Wire.begin(PMU_I2C_SDA, PMU_I2C_SCL);
    if (!pmu.begin(Wire, AXP192_SLAVE_ADDRESS))
    {
        pmu.setPowerOutPut(AXP192_LDO2, AXP202_ON); // LoRa
        pmu.setPowerOutPut(AXP192_LDO3, AXP202_ON); // GPS
        pmu.setPowerOutPut(AXP192_DCDC2, AXP202_ON);
        pmu.setPowerOutPut(AXP192_EXTEN, AXP202_ON);
        pmu.setPowerOutPut(AXP192_DCDC1, AXP202_ON); // OLED
    }
    else
    {
        CONSOLE.println("AXP192 Configuration failed");
    }

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
    gM8nDriver.Start(M8N_GGA_ENABLE | M8N_VTG_ENABLE | M8N_RMC_ENABLE);
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
            CONSOLE.println("Aborting execution : Verify connection to CC1101 board");
            CONSOLE.println("Halted");
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
    if ((firstLoop) && (gConfiguration.eeprom.networkId != 0))
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

void PrintNetworkMap(MicronetCodec::NetworkMap *networkMap)
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

void PrintMessageFifo(MicronetMessageFifo &messageFifo)
{
    if (messageFifo.GetNbMessages() > 0)
    {
        for (int i = 0; i < messageFifo.GetNbMessages(); i++)
        {
            MicronetMessage_t *message = messageFifo.Peek(i);
            CONSOLE.print("MSG ");
            CONSOLE.print(i);
            CONSOLE.print(" : ");
            CONSOLE.print(message->startTime_us);
            CONSOLE.print("/");
            CONSOLE.print(message->len);
            CONSOLE.print(" ");
            CONSOLE.println(message->action);
        }
        CONSOLE.println("");
    }
}

void MenuAbout()
{
    CONSOLE.print("MicroNav, Version ");
    CONSOLE.print(SW_MAJOR_VERSION, DEC);
    CONSOLE.print(".");
    CONSOLE.println(SW_MINOR_VERSION, DEC);

    CONSOLE.print("Device ID : ");
    CONSOLE.println(gConfiguration.eeprom.deviceId, HEX);

    if (gConfiguration.eeprom.networkId != 0)
    {
        CONSOLE.print("Attached to Micronet Network ");
        CONSOLE.println(gConfiguration.eeprom.networkId, HEX);
    }
    else
    {
        CONSOLE.println("No Micronet Network attached");
    }

    CONSOLE.print("Wind speed factor = ");
    CONSOLE.println(gConfiguration.eeprom.windSpeedFactor_per);
    CONSOLE.print("Wind direction offset = ");
    CONSOLE.println((int)(gConfiguration.eeprom.windDirectionOffset_deg));
    CONSOLE.print("Water speed factor = ");
    CONSOLE.println(gConfiguration.eeprom.waterSpeedFactor_per);
    CONSOLE.print("Water temperature offset = ");
    CONSOLE.println((int)(gConfiguration.eeprom.waterTemperatureOffset_C));
    if (gConfiguration.ram.navCompassAvailable == false)
    {
        CONSOLE.println("No navigation compass detected, disabling magnetic heading.");
    }
    else
    {
        CONSOLE.print("Using ");
        CONSOLE.print(gNavCompass.GetDeviceName().c_str());
        CONSOLE.println(" for magnetic heading");
        CONSOLE.print("Magnetometer calibration : ");
        CONSOLE.print(gConfiguration.eeprom.xMagOffset);
        CONSOLE.print(" ");
        CONSOLE.print(gConfiguration.eeprom.yMagOffset);
        CONSOLE.print(" ");
        CONSOLE.println(gConfiguration.eeprom.zMagOffset);
    }
}

void MenuScanNetworks()
{
    MicronetMessage_t *message;
    uint32_t           nidArray[MAX_SCANNED_NETWORKS];
    int16_t            rssiArray[MAX_SCANNED_NETWORKS];

    memset(nidArray, 0, sizeof(nidArray));
    memset(rssiArray, 0, sizeof(rssiArray));

    CONSOLE.print("Scanning Micronet networks for 5 seconds ... ");

    gRxMessageFifo.ResetFifo();
    unsigned long startTime = millis();
    do
    {
        if ((message = gRxMessageFifo.Peek()) != nullptr)
        {
            // Only consider messages with a valid CRC
            if (gMicronetCodec.VerifyHeaderCrc(message))
            {
                uint32_t nid  = gMicronetCodec.GetNetworkId(message);
                int16_t  rssi = message->rssi;
                // Store the network in the array by order of reception power
                for (int i = 0; i < MAX_SCANNED_NETWORKS; i++)
                {
                    if (nidArray[i] == 0)
                    {
                        // New network
                        nidArray[i]  = nid;
                        rssiArray[i] = rssi;
                        break;
                    }
                    else if (nidArray[i] == nid)
                    {
                        // Already scanned network : update RSSI if stronger
                        if (rssi > rssiArray[i])
                        {
                            rssiArray[i] = rssi;
                        }
                        break;
                    }
                    else
                    {
                        // New network to be inserted in the list : shift the list down
                        if (rssi > rssiArray[i])
                        {
                            for (int j = (MAX_SCANNED_NETWORKS - 1); j > i; j++)
                            {
                                nidArray[j]  = nidArray[j - 1];
                                rssiArray[j] = rssiArray[j - 1];
                            }
                            nidArray[i]  = nid;
                            rssiArray[i] = rssi;
                            break;
                        }
                    }
                }
            }
            gRxMessageFifo.DeleteMessage();
        }
    } while ((millis() - startTime) < 5000);

    CONSOLE.println("done");
    CONSOLE.println("");

    // Print result
    if (nidArray[0] != 0)
    {
        CONSOLE.println("List of scanned networks :");
        CONSOLE.println("");
        for (int i = 0; i < MAX_SCANNED_NETWORKS; i++)
        {
            if (nidArray[i] != 0)
            {
                CONSOLE.print("Network ");
                CONSOLE.print(i);
                CONSOLE.print(" - ");
                CONSOLE.print(nidArray[i], HEX);
                CONSOLE.print(" (");
                if (rssiArray[i] < 70)
                    CONSOLE.print("very strong");
                else if (rssiArray[i] < 80)
                    CONSOLE.print("strong");
                else if (rssiArray[i] < 90)
                    CONSOLE.print("normal");
                else
                    CONSOLE.print("low");
                CONSOLE.println(")");
            }
        }
    }
    else
    {
        CONSOLE.println("/!\\ No Micronet network found /!\\");
        CONSOLE.println("Check that your Micronet network is powered on.");
    }
}

void MenuAttachNetwork()
{
    char     input[16], c;
    uint32_t charIndex = 0;

    CONSOLE.print("Enter Network ID to attach to : ");

    do
    {
        if (CONSOLE.available())
        {
            c = CONSOLE.read();
            if (c == 0x0d)
            {
                CONSOLE.println("");
                break;
            }
            else if ((c == 0x08) && (charIndex > 0))
            {
                charIndex--;
                CONSOLE.print(c);
                CONSOLE.print(" ");
                CONSOLE.print(c);
            }
            else if (charIndex < sizeof(input))
            {
                input[charIndex++] = c;
                CONSOLE.print(c);
            }
        };
    } while (1);

    bool     invalidInput = false;
    uint32_t newNetworkId = 0;

    if (charIndex == 0)
    {
        invalidInput = true;
    }

    for (uint32_t i = 0; i < charIndex; i++)
    {
        c = input[i];
        if ((c >= '0') && (c <= '9'))
        {
            c -= '0';
        }
        else if ((c >= 'a') && (c <= 'f'))
        {
            c = c - 'a' + 10;
        }
        else if ((c >= 'A') && (c <= 'F'))
        {
            c = c - 'A' + 10;
        }
        else
        {
            invalidInput = true;
            break;
        }

        newNetworkId = (newNetworkId << 4) | c;
    }

    if (invalidInput)
    {
        CONSOLE.println("Invalid Network ID entered, ignoring input.");
    }
    else
    {
        gConfiguration.eeprom.networkId = newNetworkId;
        CONSOLE.print("Now attached to NetworkID ");
        CONSOLE.println(newNetworkId, HEX);
        gConfiguration.SaveToEeprom();
    }
}

void MenuConvertToNmea()
{
    bool                exitNmeaLoop = false;
    MicronetMessage_t * rxMessage;
    MicronetMessageFifo txMessageFifo;
    uint32_t            lastHeadingTime = millis();

    // Check that we have been attached to a network
    if (gConfiguration.eeprom.networkId == 0)
    {
        CONSOLE.println("No Micronet network has been attached.");
        CONSOLE.println("Scan and attach a Micronet network first.");
        return;
    }

    CONSOLE.println("");
    CONSOLE.println("Starting Micronet to NMEA0183 conversion.");
    CONSOLE.println("Press ESC key at any time to stop conversion and come back to menu.");
    CONSOLE.println("");

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

            if (gMicronetCodec.navData.calibrationUpdated)
            {
                gMicronetCodec.navData.calibrationUpdated = false;
                gConfiguration.SaveCalibration(gMicronetCodec);
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
        gPanelDriver.SetNetworkStatus(gMicronetDevice.GetNetworkStatus());
        yield();

    } while (!exitNmeaLoop);

    gRfDriver.DisableFrequencyTracking();
}

void MenuScanTraffic()
{
    bool                      exitSniffLoop        = false;
    uint32_t                  lastMasterRequest_us = 0;
    MicronetCodec::NetworkMap networkMap;

    CONSOLE.println("Starting Micronet traffic scanning.");
    CONSOLE.println("Press ESC key at any time to stop scanning and come back to menu.");
    CONSOLE.println("");

    gRxMessageFifo.ResetFifo();

    MicronetMessage_t *message;
    do
    {
        if ((message = gRxMessageFifo.Peek()) != nullptr)
        {
            if (gMicronetCodec.VerifyHeaderCrc(message))
            {
                if (message->data[MICRONET_MI_OFFSET] == MICRONET_MESSAGE_ID_MASTER_REQUEST)
                {
                    CONSOLE.println("");
                    // lastMasterRequest_us = message->endTime_us;
                    // gMicronetCodec.GetNetworkMap(message, &networkMap);
                    // PrintNetworkMap(&networkMap);
                }
                PrintRawMessage(message, lastMasterRequest_us);
            }
            gRxMessageFifo.DeleteMessage();
        }

        while (CONSOLE.available() > 0)
        {
            if (CONSOLE.read() == 0x1b)
            {
                CONSOLE.println("ESC key pressed, stopping scan.");
                exitSniffLoop = true;
            }
        }
        yield();
    } while (!exitSniffLoop);
}

void MenuCalibrateMagnetoMeter()
{
    bool     exitLoop     = false;
    uint32_t pDisplayTime = 0;
    uint32_t pSampleTime  = 0;
    float    mx, my, mz;
    float    xMin = 1000;
    float    xMax = -1000;
    float    yMin = 1000;
    float    yMax = -1000;
    float    zMin = 1000;
    float    zMax = -1000;
    char     c;

    if (gConfiguration.ram.navCompassAvailable == false)
    {
        CONSOLE.println("No navigation compass detected. Exiting menu ...");
        return;
    }

    CONSOLE.println("Calibrating magnetometer ... ");

    do
    {
        uint32_t currentTime = millis();
        if ((currentTime - pSampleTime) > 100)
        {
            gNavCompass.GetMagneticField(&mx, &my, &mz);
            if ((currentTime - pDisplayTime) > 250)
            {
                pDisplayTime = currentTime;
                if (mx < xMin)
                    xMin = mx;
                if (mx > xMax)
                    xMax = mx;

                if (my < yMin)
                    yMin = my;
                if (my > yMax)
                    yMax = my;

                if (mz < zMin)
                    zMin = mz;
                if (mz > zMax)
                    zMax = mz;

                CONSOLE.print("(");
                CONSOLE.print(mx);
                CONSOLE.print(" ");
                CONSOLE.print(my);
                CONSOLE.print(" ");
                CONSOLE.print(mz);
                CONSOLE.println(")");

                CONSOLE.print("[");
                CONSOLE.print((xMin + xMax) / 2);
                CONSOLE.print(" ");
                CONSOLE.print(xMax - xMin);
                CONSOLE.print("] ");
                CONSOLE.print("[");
                CONSOLE.print((yMin + yMax) / 2);
                CONSOLE.print(" ");
                CONSOLE.print(yMax - yMin);
                CONSOLE.print("] ");
                CONSOLE.print("[");
                CONSOLE.print((zMin + zMax) / 2);
                CONSOLE.print(" ");
                CONSOLE.print(zMax - zMin);
                CONSOLE.println("]");
            }
        }

        while (CONSOLE.available() > 0)
        {
            if (CONSOLE.read() == 0x1b)
            {
                CONSOLE.println("ESC key pressed, stopping scan.");
                exitLoop = true;
            }
        }
        yield();
    } while (!exitLoop);
    CONSOLE.println("Do you want to save the new calibration values (y/n) ?");
    while (CONSOLE.available() == 0)
        ;
    c = CONSOLE.read();
    if ((c == 'y') || (c == 'Y'))
    {
        gConfiguration.eeprom.xMagOffset = (xMin + xMax) / 2;
        gConfiguration.eeprom.yMagOffset = (yMin + yMax) / 2;
        gConfiguration.eeprom.zMagOffset = (zMin + zMax) / 2;
        gConfiguration.SaveToEeprom();
        CONSOLE.println("Configuration saved");
    }
    else
    {
        CONSOLE.println("Configuration discarded");
    }
}

void MenuTestRfQuality()
{
    bool                      exitTestLoop = false;
    MicronetCodec::NetworkMap networkMap;
    float                     strength;
    TxSlotDesc_t              txSlot;
    MicronetMessage_t         txMessage;
    uint32_t                  receivedDid[MICRONET_MAX_DEVICES_PER_NETWORK];

    CONSOLE.println("Starting RF signal quality test.");
    CONSOLE.println("Press ESC key at any time to stop testing and come back to menu.");
    CONSOLE.println("");

    gRxMessageFifo.ResetFifo();

    do
    {
        MicronetMessage_t *message;

        if ((message = gRxMessageFifo.Peek()) != nullptr)
        {
            if (gMicronetCodec.VerifyHeaderCrc(message))
            {
                if (message->data[MICRONET_MI_OFFSET] == MICRONET_MESSAGE_ID_MASTER_REQUEST)
                {
                    CONSOLE.println("");
                    gMicronetCodec.GetNetworkMap(message, &networkMap);
                    txSlot = gMicronetCodec.GetAsyncTransmissionSlot(&networkMap);
                    gMicronetCodec.EncodePingMessage(&txMessage, 9, networkMap.networkId, gConfiguration.eeprom.deviceId);
                    txMessage.action       = MICRONET_ACTION_RF_TRANSMIT;
                    txMessage.startTime_us = txSlot.start_us;
                    gRfDriver.Transmit(&txMessage);
                    memset(receivedDid, 0, sizeof(receivedDid));
                }

                bool alreadyReceived = false;
                for (int i = 0; i < MICRONET_MAX_DEVICES_PER_NETWORK; i++)
                {
                    uint32_t did = gMicronetCodec.GetDeviceId(message);
                    if (receivedDid[i] == did)
                    {
                        alreadyReceived = true;
                        break;
                    }
                    else if (receivedDid[i] == 0)
                    {
                        receivedDid[i] = did;
                        break;
                    }
                }

                if (!alreadyReceived)
                {
                    PrintInt(gMicronetCodec.GetDeviceId(message));
                    CONSOLE.print(" Strength=");
                    strength = gMicronetCodec.CalculateSignalFloatStrength(message);
                    CONSOLE.print(strength);
                    CONSOLE.print(" (");
                    if (strength < 1.0)
                    {
                        CONSOLE.print("Very low");
                    }
                    else if (strength < 2.5)
                    {
                        CONSOLE.print("Low");
                    }
                    else if (strength < 5.0)
                    {
                        CONSOLE.print("Medium");
                    }
                    else if (strength < 7.0)
                    {
                        CONSOLE.print("Good");
                    }
                    else if (strength < 9.0)
                    {
                        CONSOLE.print("Very Good");
                    }
                    else
                    {
                        CONSOLE.print("Excellent");
                    }

                    CONSOLE.print(") ");
                    switch (gMicronetCodec.GetDeviceType(message))
                    {
                    case MICRONET_DEVICE_TYPE_HULL_TRANSMITTER:
                        CONSOLE.print("Hull Transmitter");
                        break;
                    case MICRONET_DEVICE_TYPE_WIND_TRANSDUCER:
                        CONSOLE.print("Wind Transducer");
                        break;
                    case MICRONET_DEVICE_TYPE_NMEA_CONVERTER:
                        CONSOLE.print("NMEA Converter");
                        break;
                    case MICRONET_DEVICE_TYPE_MAST_ROTATION:
                        CONSOLE.print("Mast Rotation Sensor");
                        break;
                    case MICRONET_DEVICE_TYPE_MOB:
                        CONSOLE.print("MOB");
                        break;
                    case MICRONET_DEVICE_TYPE_SDPOD:
                        CONSOLE.print("SDPOD");
                        break;
                    case MICRONET_DEVICE_TYPE_DUAL_DISPLAY:
                        CONSOLE.print("Dual Display");
                        break;
                    case MICRONET_DEVICE_TYPE_ANALOG_WIND_DISPLAY:
                        CONSOLE.print("Analog Wind Display");
                        break;
                    case MICRONET_DEVICE_TYPE_DUAL_MAXI_DISPLAY:
                        CONSOLE.print("Dual Maxi Display");
                        break;
                    case MICRONET_DEVICE_TYPE_REMOTE_DISPLAY:
                        CONSOLE.print("Remote Display");
                        break;
                    default:
                        CONSOLE.print("Unknown device");
                        break;
                    }
                    if (networkMap.masterDevice == gMicronetCodec.GetDeviceId(message))
                    {
                        CONSOLE.print(", MASTER");
                    }
                    CONSOLE.println("");
                }
            }
            gRxMessageFifo.DeleteMessage();
        }

        while (CONSOLE.available() > 0)
        {
            if (CONSOLE.read() == 0x1b)
            {
                CONSOLE.println("ESC key pressed, stopping scan.");
                exitTestLoop = true;
            }
        }
        yield();
    } while (!exitTestLoop);
}
