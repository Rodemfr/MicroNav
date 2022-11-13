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

/***************************************************************************/
/*                              Includes                                   */
/***************************************************************************/

#include <Arduino.h>

/***************************************************************************/
/*                              Constants                                  */
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

/***************************************************************************/
/*                              Functions                                  */
/***************************************************************************/

#define GPS_RX_PIN 34
#define GPS_TX_PIN 12
#define BUTTON_PIN 38
#define BUTTON_PIN_MASK GPIO_SEL_38

#define I2C_SDA                     21
#define I2C_SCL                     22
#define PMU_IRQ                     35

#define RADIO_SCLK_PIN               5
#define RADIO_MISO_PIN              19
#define RADIO_MOSI_PIN              27
#define RADIO_CS_PIN                18
#define RADIO_DI0_PIN               26
#define RADIO_RST_PIN               23
#define RADIO_DIO1_PIN              33
#define RADIO_BUSY_PIN              32

#define GPS_BAND_RATE      9600

// bool initPMU()
// {
//     Wire.begin(I2C_SDA, I2C_SCL);

//     if (PMU.begin(Wire, AXP192_SLAVE_ADDRESS) == AXP_FAIL) {
//         return false;
//     }
//     /*
//      * The charging indicator can be turned on or off
//      * * * */
//     // PMU.setChgLEDMode(LED_BLINK_4HZ);

//     /*
//     * The default ESP32 power supply has been turned on,
//     * no need to set, please do not set it, if it is turned off,
//     * it will not be able to program
//     *
//     *   PMU.setDCDC1Voltage(3300);
//     *   PMU.setPowerOutPut(AXP192_DCDC1, AXP202_ON);
//     *
//     * * * */

//     /*
//      *   Turn off unused power sources to save power
//      * **/
//     PMU.setPowerOutPut(AXP192_DCDC2, AXP202_OFF);
//     PMU.setPowerOutPut(AXP192_LDO2, AXP202_OFF);
//     PMU.setPowerOutPut(AXP192_LDO3, AXP202_OFF);
//     PMU.setPowerOutPut(AXP192_EXTEN, AXP202_OFF);

//     /*
//      * Set the power of LoRa and GPS module to 3.3V
//      **/
//     PMU.setLDO2Voltage(3300);   //LoRa VDD
//     PMU.setLDO3Voltage(3300);   //GPS  VDD

//     PMU.setPowerOutPut(AXP192_LDO2, AXP202_ON);
//     PMU.setPowerOutPut(AXP192_LDO3, AXP202_ON);

//     return true;
// }

// void initBoard()
// {
//     Serial.begin(9600);
//     Serial.println("initBoard");
//     Serial1.begin(GPS_BAND_RATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
//     SPI.begin(RADIO_SCLK_PIN, RADIO_MISO_PIN, RADIO_MOSI_PIN, RADIO_CS_PIN);
//     initPMU();
// }

void setup()
{
    // initBoard();
    
    // // When the power is turned on, a delay is required.
    // delay(1500);

    // Serial.println(F("DeviceExample.ino"));
    // Serial.println(F("A simple demonstration of TinyGPS++ with an attached GPS module"));
    // Serial.print(F("Testing TinyGPS++ library v. "));
    // Serial.println(F("by Mikal Hart"));
    // Serial.println();

      // initialize digital pin LED_BUILTIN as an output.
    Serial.begin(115200);
}

void loop()
{
    delay(1000);                       // wait for a second
    Serial.println("initBoard");
}
