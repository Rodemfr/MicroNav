/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Driver for SX1276                                             *
 * Author:   Ronan Demoment                                                *
 *           heavily based on RadioLib driver by Jan Gromes                *
 *           (https://github.com/jgromes/RadioLib)                         *
 *                                                                         *
 ***************************************************************************
 *   Copyright (C) 2022 by Ronan Demoment                                  *
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

#ifndef SX1276REGS_H_
#define SX1276REGS_H_

/***************************************************************************/
/*                              Includes                                   */
/***************************************************************************/

#include <Arduino.h>
#include <SPI.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

// SX1278 specific register map
#define SX1276_REG_MODEM_CONFIG_3                     0x26
#define SX1276_REG_PLL_HOP                            0x44
#define SX1276_REG_TCXO                               0x4B
#define SX1276_REG_PA_DAC                             0x4D
#define SX1276_REG_FORMER_TEMP                        0x5B
#define SX1276_REG_REG_BIT_RATE_FRAC                  0x5D
#define SX1276_REG_AGC_REF                            0x61
#define SX1276_REG_AGC_THRESH_1                       0x62
#define SX1276_REG_AGC_THRESH_2                       0x63
#define SX1276_REG_AGC_THRESH_3                       0x64
#define SX1276_REG_PLL                                0x70

// SX1278 LoRa modem settings
// SX1276_REG_OP_MODE                                                  MSB   LSB   DESCRIPTION
#define SX1276_HIGH_FREQ                              0b00000000  //  3     3     access HF test registers
#define SX1276_LOW_FREQ                               0b00001000  //  3     3     access LF test registers

// SX1276_REG_FRF_MSB + REG_FRF_MID + REG_FRF_LSB
#define SX1276_FRF_MSB                                0x6C        //  7     0     carrier frequency setting: f_RF = (F(XOSC) * FRF)/2^19
#define SX1276_FRF_MID                                0x80        //  7     0         where F(XOSC) = 32 MHz
#define SX1276_FRF_LSB                                0x00        //  7     0               FRF = 3 byte value of FRF registers

// SX1276_REG_PA_CONFIG
#define SX1276_MAX_POWER                              0b01110000  //  6     4     max power: P_max = 10.8 + 0.6*MAX_POWER [dBm]; P_max(MAX_POWER = 0b111) = 15 dBm
#define SX1276_LOW_POWER                              0b00100000  //  6     4

// SX1276_REG_LNA
#define SX1276_LNA_BOOST_LF_OFF                       0b00000000  //  4     3     default LNA current

// SX127X_REG_MODEM_CONFIG_1
#define SX1276_BW_7_80_KHZ                            0b00000000  //  7     4     bandwidth:  7.80 kHz
#define SX1276_BW_10_40_KHZ                           0b00010000  //  7     4                 10.40 kHz
#define SX1276_BW_15_60_KHZ                           0b00100000  //  7     4                 15.60 kHz
#define SX1276_BW_20_80_KHZ                           0b00110000  //  7     4                 20.80 kHz
#define SX1276_BW_31_25_KHZ                           0b01000000  //  7     4                 31.25 kHz
#define SX1276_BW_41_70_KHZ                           0b01010000  //  7     4                 41.70 kHz
#define SX1276_BW_62_50_KHZ                           0b01100000  //  7     4                 62.50 kHz
#define SX1276_BW_125_00_KHZ                          0b01110000  //  7     4                 125.00 kHz
#define SX1276_BW_250_00_KHZ                          0b10000000  //  7     4                 250.00 kHz
#define SX1276_BW_500_00_KHZ                          0b10010000  //  7     4                 500.00 kHz
#define SX1276_CR_4_5                                 0b00000010  //  3     1     error coding rate:  4/5
#define SX1276_CR_4_6                                 0b00000100  //  3     1                         4/6
#define SX1276_CR_4_7                                 0b00000110  //  3     1                         4/7
#define SX1276_CR_4_8                                 0b00001000  //  3     1                         4/8
#define SX1276_HEADER_EXPL_MODE                       0b00000000  //  0     0     explicit header mode
#define SX1276_HEADER_IMPL_MODE                       0b00000001  //  0     0     implicit header mode

// SX127X_REG_MODEM_CONFIG_2
#define SX1276_RX_CRC_MODE_OFF                        0b00000000  //  2     2     CRC disabled
#define SX1276_RX_CRC_MODE_ON                         0b00000100  //  2     2     CRC enabled

// SX1276_REG_MODEM_CONFIG_3
#define SX1276_LOW_DATA_RATE_OPT_OFF                  0b00000000  //  3     3     low data rate optimization disabled
#define SX1276_LOW_DATA_RATE_OPT_ON                   0b00001000  //  3     3     low data rate optimization enabled
#define SX1276_AGC_AUTO_OFF                           0b00000000  //  2     2     LNA gain set by REG_LNA
#define SX1276_AGC_AUTO_ON                            0b00000100  //  2     2     LNA gain set by internal AGC loop

// SX127X_REG_VERSION
#define SX1276_CHIP_VERSION                           0x12

// SX1278 FSK modem settings
// SX127X_REG_PA_RAMP
#define SX1276_NO_SHAPING                             0b00000000  //  6     5     data shaping: no shaping (default)
#define SX1276_FSK_GAUSSIAN_1_0                       0b00100000  //  6     5                   FSK modulation Gaussian filter, BT = 1.0
#define SX1276_FSK_GAUSSIAN_0_5                       0b01000000  //  6     5                   FSK modulation Gaussian filter, BT = 0.5
#define SX1276_FSK_GAUSSIAN_0_3                       0b01100000  //  6     5                   FSK modulation Gaussian filter, BT = 0.3
#define SX1276_OOK_FILTER_BR                          0b00100000  //  6     5                   OOK modulation filter, f_cutoff = BR
#define SX1276_OOK_FILTER_2BR                         0b01000000  //  6     5                   OOK modulation filter, f_cutoff = 2*BR

// SX1276_REG_AGC_REF
#define SX1276_AGC_REFERENCE_LEVEL_LF                 0x19        //  5     0     floor reference for AGC thresholds: AgcRef = -174 + 10*log(2*RxBw) + 8 + AGC_REFERENCE_LEVEL [dBm]: below 525 MHz
#define SX1276_AGC_REFERENCE_LEVEL_HF                 0x1C        //  5     0                                                                                                         above 779 MHz

// SX1276_REG_AGC_THRESH_1
#define SX1276_AGC_STEP_1_LF                          0x0C        //  4     0     1st AGC threshold: below 525 MHz
#define SX1276_AGC_STEP_1_HF                          0x0E        //  4     0                        above 779 MHz

// SX1276_REG_AGC_THRESH_2
#define SX1276_AGC_STEP_2_LF                          0x40        //  7     4     2nd AGC threshold: below 525 MHz
#define SX1276_AGC_STEP_2_HF                          0x50        //  7     4                        above 779 MHz
#define SX1276_AGC_STEP_3                             0x0B        //  3     0     3rd AGC threshold

// SX1276_REG_AGC_THRESH_3
#define SX1276_AGC_STEP_4                             0xC0        //  7     4     4th AGC threshold
#define SX1276_AGC_STEP_5                             0x0C        //  4     0     5th AGC threshold

/***************************************************************************/
/*                                Types                                    */
/***************************************************************************/

/***************************************************************************/
/*                               Classes                                   */
/***************************************************************************/

#endif
