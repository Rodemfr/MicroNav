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

// SX127x physical layer properties
#define SX127X_FREQUENCY_STEP_SIZE                    61.03515625
#define SX127X_MAX_PACKET_LENGTH                      255
#define SX127X_MAX_PACKET_LENGTH_FSK                  64
#define SX127X_CRYSTAL_FREQ                           32.0
#define SX127X_DIV_EXPONENT                           19
#define SX127X_FIFO_MAX_SIZE                          64

// SX127x series common LoRa registers
#define SX127X_REG_FIFO                               0x00
#define SX127X_REG_OP_MODE                            0x01
#define SX127X_REG_FRF_MSB                            0x06
#define SX127X_REG_FRF_MID                            0x07
#define SX127X_REG_FRF_LSB                            0x08
#define SX127X_REG_PA_CONFIG                          0x09
#define SX127X_REG_PA_RAMP                            0x0A
#define SX127X_REG_OCP                                0x0B
#define SX127X_REG_LNA                                0x0C
#define SX127X_REG_FIFO_ADDR_PTR                      0x0D
#define SX127X_REG_FIFO_TX_BASE_ADDR                  0x0E
#define SX127X_REG_FIFO_RX_BASE_ADDR                  0x0F
#define SX127X_REG_FIFO_RX_CURRENT_ADDR               0x10
#define SX127X_REG_IRQ_FLAGS_MASK                     0x11
#define SX127X_REG_IRQ_FLAGS                          0x12
#define SX127X_REG_RX_NB_BYTES                        0x13
#define SX127X_REG_RX_HEADER_CNT_VALUE_MSB            0x14
#define SX127X_REG_RX_HEADER_CNT_VALUE_LSB            0x15
#define SX127X_REG_RX_PACKET_CNT_VALUE_MSB            0x16
#define SX127X_REG_RX_PACKET_CNT_VALUE_LSB            0x17
#define SX127X_REG_MODEM_STAT                         0x18
#define SX127X_REG_PKT_SNR_VALUE                      0x19
#define SX127X_REG_PKT_RSSI_VALUE                     0x1A
#define SX127X_REG_RSSI_VALUE                         0x1B
#define SX127X_REG_HOP_CHANNEL                        0x1C
#define SX127X_REG_MODEM_CONFIG_1                     0x1D
#define SX127X_REG_MODEM_CONFIG_2                     0x1E
#define SX127X_REG_SYMB_TIMEOUT_LSB                   0x1F
#define SX127X_REG_PREAMBLE_MSB                       0x20
#define SX127X_REG_PREAMBLE_LSB                       0x21
#define SX127X_REG_PAYLOAD_LENGTH                     0x22
#define SX127X_REG_MAX_PAYLOAD_LENGTH                 0x23
#define SX127X_REG_HOP_PERIOD                         0x24
#define SX127X_REG_FIFO_RX_BYTE_ADDR                  0x25
#define SX127X_REG_FEI_MSB                            0x28
#define SX127X_REG_FEI_MID                            0x29
#define SX127X_REG_FEI_LSB                            0x2A
#define SX127X_REG_RSSI_WIDEBAND                      0x2C
#define SX127X_REG_DETECT_OPTIMIZE                    0x31
#define SX127X_REG_INVERT_IQ                          0x33
#define SX127X_REG_DETECTION_THRESHOLD                0x37
#define SX127X_REG_SYNC_WORD                          0x39
#define SX127X_REG_INVERT_IQ2                         0x3B
#define SX127X_REG_DIO_MAPPING_1                      0x40
#define SX127X_REG_DIO_MAPPING_2                      0x41
#define SX127X_REG_VERSION                            0x42

// SX127x common LoRa modem settings
// SX127X_REG_OP_MODE                                                 MSB   LSB   DESCRIPTION
#define SX127X_FSK_OOK                                0b00000000  //  7     7     FSK/OOK mode
#define SX127X_LORA                                   0b10000000  //  7     7     LoRa mode
#define SX127X_ACCESS_SHARED_REG_OFF                  0b00000000  //  6     6     access LoRa registers (0x0D:0x3F) in LoRa mode
#define SX127X_ACCESS_SHARED_REG_ON                   0b01000000  //  6     6     access FSK registers (0x0D:0x3F) in LoRa mode
#define SX127X_SLEEP                                  0b00000000  //  2     0     sleep
#define SX127X_STANDBY                                0b00000001  //  2     0     standby
#define SX127X_FSTX                                   0b00000010  //  2     0     frequency synthesis TX
#define SX127X_TX                                     0b00000011  //  2     0     transmit
#define SX127X_FSRX                                   0b00000100  //  2     0     frequency synthesis RX
#define SX127X_RXCONTINUOUS                           0b00000101  //  2     0     receive continuous
#define SX127X_RXSINGLE                               0b00000110  //  2     0     receive single
#define SX127X_CAD                                    0b00000111  //  2     0     channel activity detection

// SX127X_REG_PA_CONFIG
#define SX127X_PA_SELECT_RFO                          0b00000000  //  7     7     RFO pin output, power limited to +14 dBm
#define SX127X_PA_SELECT_BOOST                        0b10000000  //  7     7     PA_BOOST pin output, power limited to +20 dBm
#define SX127X_OUTPUT_POWER                           0b00001111  //  3     0     output power: P_out = 2 + OUTPUT_POWER [dBm] for PA_SELECT_BOOST
                                                                  //                            P_out = -1 + OUTPUT_POWER [dBm] for PA_SELECT_RFO

// SX127X_REG_OCP
#define SX127X_OCP_OFF                                0b00000000  //  5     5     PA overload current protection disabled
#define SX127X_OCP_ON                                 0b00100000  //  5     5     PA overload current protection enabled
#define SX127X_OCP_TRIM                               0b00001011  //  4     0     OCP current: I_max(OCP_TRIM = 0b1011) = 100 mA

// SX127X_REG_LNA
#define SX127X_LNA_GAIN_1                             0b00100000  //  7     5     LNA gain setting:   max gain
#define SX127X_LNA_GAIN_2                             0b01000000  //  7     5                         .
#define SX127X_LNA_GAIN_3                             0b01100000  //  7     5                         .
#define SX127X_LNA_GAIN_4                             0b10000000  //  7     5                         .
#define SX127X_LNA_GAIN_5                             0b10100000  //  7     5                         .
#define SX127X_LNA_GAIN_6                             0b11000000  //  7     5                         min gain
#define SX127X_LNA_BOOST_OFF                          0b00000000  //  1     0     default LNA current
#define SX127X_LNA_BOOST_ON                           0b00000011  //  1     0     150% LNA current

// SX127X_REG_MODEM_CONFIG_2
#define SX127X_SF_6                                   0b01100000  //  7     4     spreading factor:   64 chips/bit
#define SX127X_SF_7                                   0b01110000  //  7     4                         128 chips/bit
#define SX127X_SF_8                                   0b10000000  //  7     4                         256 chips/bit
#define SX127X_SF_9                                   0b10010000  //  7     4                         512 chips/bit
#define SX127X_SF_10                                  0b10100000  //  7     4                         1024 chips/bit
#define SX127X_SF_11                                  0b10110000  //  7     4                         2048 chips/bit
#define SX127X_SF_12                                  0b11000000  //  7     4                         4096 chips/bit
#define SX127X_TX_MODE_SINGLE                         0b00000000  //  3     3     single TX
#define SX127X_TX_MODE_CONT                           0b00001000  //  3     3     continuous TX
#define SX127X_RX_TIMEOUT_MSB                         0b00000000  //  1     0

// SX127X_REG_SYMB_TIMEOUT_LSB
#define SX127X_RX_TIMEOUT_LSB                         0b01100100  //  7     0     10 bit RX operation timeout

// SX127X_REG_PREAMBLE_MSB + REG_PREAMBLE_LSB
#define SX127X_PREAMBLE_LENGTH_MSB                    0b00000000  //  7     0     2 byte preamble length setting: l_P = PREAMBLE_LENGTH + 4.25
#define SX127X_PREAMBLE_LENGTH_LSB                    0b00001000  //  7     0         where l_p = preamble length

// SX127X_REG_DETECT_OPTIMIZE
#define SX127X_DETECT_OPTIMIZE_SF_6                   0b00000101  //  2     0     SF6 detection optimization
#define SX127X_DETECT_OPTIMIZE_SF_7_12                0b00000011  //  2     0     SF7 to SF12 detection optimization

// SX127X_REG_INVERT_IQ
#define SX127X_INVERT_IQ_RXPATH_ON                    0b01000000  //  6     6     I and Q signals are inverted
#define SX127X_INVERT_IQ_RXPATH_OFF                   0b00000000  //  6     6     normal mode
#define SX127X_INVERT_IQ_TXPATH_ON                    0b00000001  //  0     0     I and Q signals are inverted
#define SX127X_INVERT_IQ_TXPATH_OFF                   0b00000000  //  0     0     normal mode

// SX127X_REG_DETECTION_THRESHOLD
#define SX127X_DETECTION_THRESHOLD_SF_6               0b00001100  //  7     0     SF6 detection threshold
#define SX127X_DETECTION_THRESHOLD_SF_7_12            0b00001010  //  7     0     SF7 to SF12 detection threshold

// SX127X_REG_PA_DAC
#define SX127X_PA_BOOST_OFF                           0b00000100  //  2     0     PA_BOOST disabled
#define SX127X_PA_BOOST_ON                            0b00000111  //  2     0     +20 dBm on PA_BOOST when OUTPUT_POWER = 0b1111

// SX127X_REG_HOP_PERIOD
#define SX127X_HOP_PERIOD_OFF                         0b00000000  //  7     0     number of periods between frequency hops; 0 = disabled
#define SX127X_HOP_PERIOD_MAX                         0b11111111  //  7     0

// SX127X_REG_IRQ_FLAGS
#define SX127X_CLEAR_IRQ_FLAG_RX_TIMEOUT              0b10000000  //  7     7     timeout
#define SX127X_CLEAR_IRQ_FLAG_RX_DONE                 0b01000000  //  6     6     packet reception complete
#define SX127X_CLEAR_IRQ_FLAG_PAYLOAD_CRC_ERROR       0b00100000  //  5     5     payload CRC error
#define SX127X_CLEAR_IRQ_FLAG_VALID_HEADER            0b00010000  //  4     4     valid header received
#define SX127X_CLEAR_IRQ_FLAG_TX_DONE                 0b00001000  //  3     3     payload transmission complete
#define SX127X_CLEAR_IRQ_FLAG_CAD_DONE                0b00000100  //  2     2     CAD complete
#define SX127X_CLEAR_IRQ_FLAG_FHSS_CHANGE_CHANNEL     0b00000010  //  1     1     FHSS change channel
#define SX127X_CLEAR_IRQ_FLAG_CAD_DETECTED            0b00000001  //  0     0     valid LoRa signal detected during CAD operation

// SX127X_REG_IRQ_FLAGS_MASK
#define SX127X_MASK_IRQ_FLAG_RX_TIMEOUT               0b01111111  //  7     7     timeout
#define SX127X_MASK_IRQ_FLAG_RX_DONE                  0b10111111  //  6     6     packet reception complete
#define SX127X_MASK_IRQ_FLAG_PAYLOAD_CRC_ERROR        0b11011111  //  5     5     payload CRC error
#define SX127X_MASK_IRQ_FLAG_VALID_HEADER             0b11101111  //  4     4     valid header received
#define SX127X_MASK_IRQ_FLAG_TX_DONE                  0b11110111  //  3     3     payload transmission complete
#define SX127X_MASK_IRQ_FLAG_CAD_DONE                 0b11111011  //  2     2     CAD complete
#define SX127X_MASK_IRQ_FLAG_FHSS_CHANGE_CHANNEL      0b11111101  //  1     1     FHSS change channel
#define SX127X_MASK_IRQ_FLAG_CAD_DETECTED             0b11111110  //  0     0     valid LoRa signal detected during CAD operation

// SX127X_REG_FIFO_TX_BASE_ADDR
#define SX127X_FIFO_TX_BASE_ADDR_MAX                  0b00000000  //  7     0     allocate the entire FIFO buffer for TX only

// SX127X_REG_FIFO_RX_BASE_ADDR
#define SX127X_FIFO_RX_BASE_ADDR_MAX                  0b00000000  //  7     0     allocate the entire FIFO buffer for RX only

// SX127X_REG_SYNC_WORD
#define SX127X_SYNC_WORD                              0x12        //  7     0     default LoRa sync word
#define SX127X_SYNC_WORD_LORAWAN                      0x34        //  7     0     sync word reserved for LoRaWAN networks

// SX127X_REG_INVERT_IQ2
#define SX127X_IQ2_ENABLE                             0x19        //  7     0     enable optimize for inverted IQ
#define SX127X_IQ2_DISABLE                            0x1D        //  7     0     reset optimize for inverted IQ

// SX127x series common FSK registers
// NOTE: FSK register names that are conflicting with LoRa registers are marked with "_FSK" suffix
#define SX127X_REG_BITRATE_MSB                        0x02
#define SX127X_REG_BITRATE_LSB                        0x03
#define SX127X_REG_FDEV_MSB                           0x04
#define SX127X_REG_FDEV_LSB                           0x05
#define SX127X_REG_RX_CONFIG                          0x0D
#define SX127X_REG_RSSI_CONFIG                        0x0E
#define SX127X_REG_RSSI_COLLISION                     0x0F
#define SX127X_REG_RSSI_THRESH                        0x10
#define SX127X_REG_RSSI_VALUE_FSK                     0x11
#define SX127X_REG_RX_BW                              0x12
#define SX127X_REG_AFC_BW                             0x13
#define SX127X_REG_OOK_PEAK                           0x14
#define SX127X_REG_OOK_FIX                            0x15
#define SX127X_REG_OOK_AVG                            0x16
#define SX127X_REG_AFC_FEI                            0x1A
#define SX127X_REG_AFC_MSB                            0x1B
#define SX127X_REG_AFC_LSB                            0x1C
#define SX127X_REG_FEI_MSB_FSK                        0x1D
#define SX127X_REG_FEI_LSB_FSK                        0x1E
#define SX127X_REG_PREAMBLE_DETECT                    0x1F
#define SX127X_REG_RX_TIMEOUT_1                       0x20
#define SX127X_REG_RX_TIMEOUT_2                       0x21
#define SX127X_REG_RX_TIMEOUT_3                       0x22
#define SX127X_REG_RX_DELAY                           0x23
#define SX127X_REG_OSC                                0x24
#define SX127X_REG_PREAMBLE_MSB_FSK                   0x25
#define SX127X_REG_PREAMBLE_LSB_FSK                   0x26
#define SX127X_REG_SYNC_CONFIG                        0x27
#define SX127X_REG_SYNC_VALUE_1                       0x28
#define SX127X_REG_SYNC_VALUE_2                       0x29
#define SX127X_REG_SYNC_VALUE_3                       0x2A
#define SX127X_REG_SYNC_VALUE_4                       0x2B
#define SX127X_REG_SYNC_VALUE_5                       0x2C
#define SX127X_REG_SYNC_VALUE_6                       0x2D
#define SX127X_REG_SYNC_VALUE_7                       0x2E
#define SX127X_REG_SYNC_VALUE_8                       0x2F
#define SX127X_REG_PACKET_CONFIG_1                    0x30
#define SX127X_REG_PACKET_CONFIG_2                    0x31
#define SX127X_REG_PAYLOAD_LENGTH_FSK                 0x32
#define SX127X_REG_NODE_ADRS                          0x33
#define SX127X_REG_BROADCAST_ADRS                     0x34
#define SX127X_REG_FIFO_THRESH                        0x35
#define SX127X_REG_SEQ_CONFIG_1                       0x36
#define SX127X_REG_SEQ_CONFIG_2                       0x37
#define SX127X_REG_TIMER_RESOL                        0x38
#define SX127X_REG_TIMER1_COEF                        0x39
#define SX127X_REG_TIMER2_COEF                        0x3A
#define SX127X_REG_IMAGE_CAL                          0x3B
#define SX127X_REG_TEMP                               0x3C
#define SX127X_REG_LOW_BAT                            0x3D
#define SX127X_REG_IRQ_FLAGS_1                        0x3E
#define SX127X_REG_IRQ_FLAGS_2                        0x3F

// SX127x common FSK modem settings
// SX127X_REG_OP_MODE
#define SX127X_MODULATION_FSK                         0b00000000  //  6     5     FSK modulation scheme
#define SX127X_MODULATION_OOK                         0b00100000  //  6     5     OOK modulation scheme
#define SX127X_RX                                     0b00000101  //  2     0     receiver mode

// SX127X_REG_BITRATE_MSB + SX127X_REG_BITRATE_LSB
#define SX127X_BITRATE_MSB                            0x1A        //  7     0     bit rate setting: BitRate = F(XOSC)/(BITRATE + BITRATE_FRAC/16)
#define SX127X_BITRATE_LSB                            0x0B        //  7     0                       default value: 4.8 kbps

// SX127X_REG_FDEV_MSB + SX127X_REG_FDEV_LSB
#define SX127X_FDEV_MSB                               0x00        //  5     0     frequency deviation: Fdev = Fstep * FDEV
#define SX127X_FDEV_LSB                               0x52        //  7     0                          default value: 5 kHz

// SX127X_REG_RX_CONFIG
#define SX127X_RESTART_RX_ON_COLLISION_OFF            0b00000000  //  7     7     automatic receiver restart disabled (default)
#define SX127X_RESTART_RX_ON_COLLISION_ON             0b10000000  //  7     7     automatically restart receiver if it gets saturated or on packet collision
#define SX127X_RESTART_RX_WITHOUT_PLL_LOCK            0b01000000  //  6     6     manually restart receiver without frequency change
#define SX127X_RESTART_RX_WITH_PLL_LOCK               0b00100000  //  5     5     manually restart receiver with frequency change
#define SX127X_AFC_AUTO_OFF                           0b00000000  //  4     4     no AFC performed (default)
#define SX127X_AFC_AUTO_ON                            0b00010000  //  4     4     AFC performed at each receiver startup
#define SX127X_AGC_AUTO_OFF                           0b00000000  //  3     3     LNA gain set manually by register
#define SX127X_AGC_AUTO_ON                            0b00001000  //  3     3     LNA gain controlled by AGC
#define SX127X_RX_TRIGGER_NONE                        0b00000000  //  2     0     receiver startup at: none
#define SX127X_RX_TRIGGER_RSSI_INTERRUPT              0b00000001  //  2     0                          RSSI interrupt
#define SX127X_RX_TRIGGER_PREAMBLE_DETECT             0b00000110  //  2     0                          preamble detected
#define SX127X_RX_TRIGGER_BOTH                        0b00000111  //  2     0                          RSSI interrupt and preamble detected

// SX127X_REG_RSSI_CONFIG
#define SX127X_RSSI_SMOOTHING_SAMPLES_2               0b00000000  //  2     0     number of samples for RSSI average: 2
#define SX127X_RSSI_SMOOTHING_SAMPLES_4               0b00000001  //  2     0                                         4
#define SX127X_RSSI_SMOOTHING_SAMPLES_8               0b00000010  //  2     0                                         8 (default)
#define SX127X_RSSI_SMOOTHING_SAMPLES_16              0b00000011  //  2     0                                         16
#define SX127X_RSSI_SMOOTHING_SAMPLES_32              0b00000100  //  2     0                                         32
#define SX127X_RSSI_SMOOTHING_SAMPLES_64              0b00000101  //  2     0                                         64
#define SX127X_RSSI_SMOOTHING_SAMPLES_128             0b00000110  //  2     0                                         128
#define SX127X_RSSI_SMOOTHING_SAMPLES_256             0b00000111  //  2     0                                         256

// SX127X_REG_RSSI_COLLISION
#define SX127X_RSSI_COLLISION_THRESHOLD               0x0A        //  7     0     RSSI threshold in dB that will be considered a collision, default value: 10 dB

// SX127X_REG_RSSI_THRESH
#define SX127X_RSSI_THRESHOLD                         0xFF        //  7     0     RSSI threshold that will trigger RSSI interrupt, RssiThreshold = RSSI_THRESHOLD / 2 [dBm]

// SX127X_REG_RX_BW
#define SX127X_RX_BW_MANT_16                          0b00000000  //  4     3     channel filter bandwidth: RxBw = F(XOSC) / (RxBwMant * 2^(RxBwExp + 2)) [kHz]
#define SX127X_RX_BW_MANT_20                          0b00001000  //  4     3
#define SX127X_RX_BW_MANT_24                          0b00010000  //  4     3     default RxBwMant parameter
#define SX127X_RX_BW_EXP                              0b00000101  //  2     0     default RxBwExp parameter

// SX127X_REG_AFC_BW
#define SX127X_RX_BW_MANT_AFC                         0b00001000  //  4     3     default RxBwMant parameter used during AFC
#define SX127X_RX_BW_EXP_AFC                          0b00000011  //  2     0     default RxBwExp parameter used during AFC

// SX127X_REG_OOK_PEAK
#define SX127X_BIT_SYNC_OFF                           0b00000000  //  5     5     bit synchronizer disabled (not allowed in packet mode)
#define SX127X_BIT_SYNC_ON                            0b00100000  //  5     5     bit synchronizer enabled (default)
#define SX127X_OOK_THRESH_FIXED                       0b00000000  //  4     3     OOK threshold type: fixed value
#define SX127X_OOK_THRESH_PEAK                        0b00001000  //  4     3                         peak mode (default)
#define SX127X_OOK_THRESH_AVERAGE                     0b00010000  //  4     3                         average mode
#define SX127X_OOK_PEAK_THRESH_STEP_0_5_DB            0b00000000  //  2     0     OOK demodulator step size: 0.5 dB (default)
#define SX127X_OOK_PEAK_THRESH_STEP_1_0_DB            0b00000001  //  2     0                                1.0 dB
#define SX127X_OOK_PEAK_THRESH_STEP_1_5_DB            0b00000010  //  2     0                                1.5 dB
#define SX127X_OOK_PEAK_THRESH_STEP_2_0_DB            0b00000011  //  2     0                                2.0 dB
#define SX127X_OOK_PEAK_THRESH_STEP_3_0_DB            0b00000100  //  2     0                                3.0 dB
#define SX127X_OOK_PEAK_THRESH_STEP_4_0_DB            0b00000101  //  2     0                                4.0 dB
#define SX127X_OOK_PEAK_THRESH_STEP_5_0_DB            0b00000110  //  2     0                                5.0 dB
#define SX127X_OOK_PEAK_THRESH_STEP_6_0_DB            0b00000111  //  2     0                                6.0 dB

// SX127X_REG_OOK_FIX
#define SX127X_OOK_FIXED_THRESHOLD                    0x0C        //  7     0     default fixed threshold for OOK data slicer

// SX127X_REG_OOK_AVG
#define SX127X_OOK_PEAK_THRESH_DEC_1_1_CHIP           0b00000000  //  7     5     OOK demodulator step period: once per chip (default)
#define SX127X_OOK_PEAK_THRESH_DEC_1_2_CHIP           0b00100000  //  7     5                                  once every 2 chips
#define SX127X_OOK_PEAK_THRESH_DEC_1_4_CHIP           0b01000000  //  7     5                                  once every 4 chips
#define SX127X_OOK_PEAK_THRESH_DEC_1_8_CHIP           0b01100000  //  7     5                                  once every 8 chips
#define SX127X_OOK_PEAK_THRESH_DEC_2_1_CHIP           0b10000000  //  7     5                                  2 times per chip
#define SX127X_OOK_PEAK_THRESH_DEC_4_1_CHIP           0b10100000  //  7     5                                  4 times per chip
#define SX127X_OOK_PEAK_THRESH_DEC_8_1_CHIP           0b11000000  //  7     5                                  8 times per chip
#define SX127X_OOK_PEAK_THRESH_DEC_16_1_CHIP          0b11100000  //  7     5                                  16 times per chip
#define SX127X_OOK_AVERAGE_OFFSET_0_DB                0b00000000  //  3     2     OOK average threshold offset: 0.0 dB (default)
#define SX127X_OOK_AVERAGE_OFFSET_2_DB                0b00000100  //  3     2                                   2.0 dB
#define SX127X_OOK_AVERAGE_OFFSET_4_DB                0b00001000  //  3     2                                   4.0 dB
#define SX127X_OOK_AVERAGE_OFFSET_6_DB                0b00001100  //  3     2                                   6.0 dB
#define SX127X_OOK_AVG_THRESH_FILT_32_PI              0b00000000  //  1     0     OOK average filter coefficient: chip rate / 32*pi
#define SX127X_OOK_AVG_THRESH_FILT_8_PI               0b00000001  //  1     0                                     chip rate / 8*pi
#define SX127X_OOK_AVG_THRESH_FILT_4_PI               0b00000010  //  1     0                                     chip rate / 4*pi (default)
#define SX127X_OOK_AVG_THRESH_FILT_2_PI               0b00000011  //  1     0                                     chip rate / 2*pi

// SX127X_REG_AFC_FEI
#define SX127X_AGC_START                              0b00010000  //  4     4     manually start AGC sequence
#define SX127X_AFC_CLEAR                              0b00000010  //  1     1     manually clear AFC register
#define SX127X_AFC_AUTO_CLEAR_OFF                     0b00000000  //  0     0     AFC register will not be cleared at the start of AFC (default)
#define SX127X_AFC_AUTO_CLEAR_ON                      0b00000001  //  0     0     AFC register will be cleared at the start of AFC

// SX127X_REG_PREAMBLE_DETECT
#define SX127X_PREAMBLE_DETECTOR_OFF                  0b00000000  //  7     7     preamble detection disabled
#define SX127X_PREAMBLE_DETECTOR_ON                   0b10000000  //  7     7     preamble detection enabled (default)
#define SX127X_PREAMBLE_DETECTOR_1_BYTE               0b00000000  //  6     5     preamble detection size: 1 byte (default)
#define SX127X_PREAMBLE_DETECTOR_2_BYTE               0b00100000  //  6     5                              2 bytes
#define SX127X_PREAMBLE_DETECTOR_3_BYTE               0b01000000  //  6     5                              3 bytes
#define SX127X_PREAMBLE_DETECTOR_TOL                  0x0A        //  4     0     default number of tolerated errors per chip (4 chips per bit)

// SX127X_REG_RX_TIMEOUT_1
#define SX127X_TIMEOUT_RX_RSSI_OFF                    0x00        //  7     0     disable receiver timeout when RSSI interrupt doesn't occur (default)

// SX127X_REG_RX_TIMEOUT_2
#define SX127X_TIMEOUT_RX_PREAMBLE_OFF                0x00        //  7     0     disable receiver timeout when preamble interrupt doesn't occur (default)

// SX127X_REG_RX_TIMEOUT_3
#define SX127X_TIMEOUT_SIGNAL_SYNC_OFF                0x00        //  7     0     disable receiver timeout when sync address interrupt doesn't occur (default)

// SX127X_REG_OSC
#define SX127X_RC_CAL_START                           0b00000000  //  3     3     manually start RC oscillator calibration
#define SX127X_CLK_OUT_FXOSC                          0b00000000  //  2     0     ClkOut frequency: F(XOSC)
#define SX127X_CLK_OUT_FXOSC_2                        0b00000001  //  2     0                       F(XOSC) / 2
#define SX127X_CLK_OUT_FXOSC_4                        0b00000010  //  2     0                       F(XOSC) / 4
#define SX127X_CLK_OUT_FXOSC_8                        0b00000011  //  2     0                       F(XOSC) / 8
#define SX127X_CLK_OUT_FXOSC_16                       0b00000100  //  2     0                       F(XOSC) / 16
#define SX127X_CLK_OUT_FXOSC_32                       0b00000101  //  2     0                       F(XOSC) / 32
#define SX127X_CLK_OUT_RC                             0b00000110  //  2     0                       RC
#define SX127X_CLK_OUT_OFF                            0b00000111  //  2     0                       disabled (default)

// SX127X_REG_PREAMBLE_MSB_FSK + SX127X_REG_PREAMBLE_LSB_FSK
#define SX127X_PREAMBLE_SIZE_MSB                      0x00        //  7     0     preamble size in bytes
#define SX127X_PREAMBLE_SIZE_LSB                      0x03        //  7     0       default value: 3 bytes

// SX127X_REG_SYNC_CONFIG
#define SX127X_AUTO_RESTART_RX_MODE_OFF               0b00000000  //  7     6     Rx mode restart after packet reception: disabled
#define SX127X_AUTO_RESTART_RX_MODE_NO_PLL            0b01000000  //  7     6                                             enabled, don't wait for PLL lock
#define SX127X_AUTO_RESTART_RX_MODE_PLL               0b10000000  //  7     6                                             enabled, wait for PLL lock (default)
#define SX127X_PREAMBLE_POLARITY_AA                   0b00000000  //  5     5     preamble polarity: 0xAA = 0b10101010 (default)
#define SX127X_PREAMBLE_POLARITY_55                   0b00100000  //  5     5                        0x55 = 0b01010101
#define SX127X_SYNC_OFF                               0b00000000  //  4     4     sync word disabled
#define SX127X_SYNC_ON                                0b00010000  //  4     4     sync word enabled (default)
#define SX127X_SYNC_SIZE                              0x03        //  2     0     sync word size in bytes, SyncSize = SYNC_SIZE + 1 bytes

// SX127X_REG_SYNC_VALUE_1 - SX127X_REG_SYNC_VALUE_8
#define SX127X_SYNC_VALUE_1                           0x01        //  7     0     sync word: 1st byte (MSB)
#define SX127X_SYNC_VALUE_2                           0x01        //  7     0                2nd byte
#define SX127X_SYNC_VALUE_3                           0x01        //  7     0                3rd byte
#define SX127X_SYNC_VALUE_4                           0x01        //  7     0                4th byte
#define SX127X_SYNC_VALUE_5                           0x01        //  7     0                5th byte
#define SX127X_SYNC_VALUE_6                           0x01        //  7     0                6th byte
#define SX127X_SYNC_VALUE_7                           0x01        //  7     0                7th byte
#define SX127X_SYNC_VALUE_8                           0x01        //  7     0                8th byte (LSB)

// SX127X_REG_PACKET_CONFIG_1
#define SX127X_PACKET_FIXED                           0b00000000  //  7     7     packet format: fixed length
#define SX127X_PACKET_VARIABLE                        0b10000000  //  7     7                    variable length (default)
#define SX127X_DC_FREE_NONE                           0b00000000  //  6     5     DC-free encoding: disabled (default)
#define SX127X_DC_FREE_MANCHESTER                     0b00100000  //  6     5                       Manchester
#define SX127X_DC_FREE_WHITENING                      0b01000000  //  6     5                       Whitening
#define SX127X_CRC_OFF                                0b00000000  //  4     4     CRC disabled
#define SX127X_CRC_ON                                 0b00010000  //  4     4     CRC enabled (default)
#define SX127X_CRC_AUTOCLEAR_OFF                      0b00001000  //  3     3     keep FIFO on CRC mismatch, issue payload ready interrupt
#define SX127X_CRC_AUTOCLEAR_ON                       0b00000000  //  3     3     clear FIFO on CRC mismatch, do not issue payload ready interrupt
#define SX127X_ADDRESS_FILTERING_OFF                  0b00000000  //  2     1     address filtering: none (default)
#define SX127X_ADDRESS_FILTERING_NODE                 0b00000010  //  2     1                        node
#define SX127X_ADDRESS_FILTERING_NODE_BROADCAST       0b00000100  //  2     1                        node or broadcast
#define SX127X_CRC_WHITENING_TYPE_CCITT               0b00000000  //  0     0     CRC and whitening algorithms: CCITT CRC with standard whitening (default)
#define SX127X_CRC_WHITENING_TYPE_IBM                 0b00000001  //  0     0                                   IBM CRC with alternate whitening

// SX127X_REG_PACKET_CONFIG_2
#define SX127X_DATA_MODE_PACKET                       0b01000000  //  6     6     data mode: packet (default)
#define SX127X_DATA_MODE_CONTINUOUS                   0b00000000  //  6     6                continuous
#define SX127X_IO_HOME_OFF                            0b00000000  //  5     5     io-homecontrol compatibility disabled (default)
#define SX127X_IO_HOME_ON                             0b00100000  //  5     5     io-homecontrol compatibility enabled

// SX127X_REG_FIFO_THRESH
#define SX127X_TX_START_FIFO_LEVEL                    0b00000000  //  7     7     start packet transmission when: number of bytes in FIFO exceeds FIFO_THRESHOLD
#define SX127X_TX_START_FIFO_NOT_EMPTY                0b10000000  //  7     7                                     at least one byte in FIFO (default)
#define SX127X_FIFO_THRESH                            0x1F        //  5     0     FIFO level threshold

// SX127X_REG_SEQ_CONFIG_1
#define SX127X_SEQUENCER_START                        0b10000000  //  7     7     manually start sequencer
#define SX127X_SEQUENCER_STOP                         0b01000000  //  6     6     manually stop sequencer
#define SX127X_IDLE_MODE_STANDBY                      0b00000000  //  5     5     chip mode during sequencer idle mode: standby (default)
#define SX127X_IDLE_MODE_SLEEP                        0b00100000  //  5     5                                           sleep
#define SX127X_FROM_START_LP_SELECTION                0b00000000  //  4     3     mode that will be set after starting sequencer: low power selection (default)
#define SX127X_FROM_START_RECEIVE                     0b00001000  //  4     3                                                     receive
#define SX127X_FROM_START_TRANSMIT                    0b00010000  //  4     3                                                     transmit
#define SX127X_FROM_START_TRANSMIT_FIFO_LEVEL         0b00011000  //  4     3                                                     transmit on a FIFO level interrupt
#define SX127X_LP_SELECTION_SEQ_OFF                   0b00000000  //  2     2     mode that will be set after exiting low power selection: sequencer off (default)
#define SX127X_LP_SELECTION_IDLE                      0b00000100  //  2     2                                                              idle state
#define SX127X_FROM_IDLE_TRANSMIT                     0b00000000  //  1     1     mode that will be set after exiting idle mode: transmit (default)
#define SX127X_FROM_IDLE_RECEIVE                      0b00000010  //  1     1                                                    receive
#define SX127X_FROM_TRANSMIT_LP_SELECTION             0b00000000  //  0     0     mode that will be set after exiting transmit mode: low power selection (default)
#define SX127X_FROM_TRANSMIT_RECEIVE                  0b00000001  //  0     0                                                        receive

// SX127X_REG_SEQ_CONFIG_2
#define SX127X_FROM_RECEIVE_PACKET_RECEIVED_PAYLOAD   0b00100000  //  7     5     mode that will be set after exiting receive mode: packet received on payload ready interrupt (default)
#define SX127X_FROM_RECEIVE_LP_SELECTION              0b01000000  //  7     5                                                       low power selection
#define SX127X_FROM_RECEIVE_PACKET_RECEIVED_CRC_OK    0b01100000  //  7     5                                                       packet received on CRC OK interrupt
#define SX127X_FROM_RECEIVE_SEQ_OFF_RSSI              0b10000000  //  7     5                                                       sequencer off on RSSI interrupt
#define SX127X_FROM_RECEIVE_SEQ_OFF_SYNC_ADDR         0b10100000  //  7     5                                                       sequencer off on sync address interrupt
#define SX127X_FROM_RECEIVE_SEQ_OFF_PREAMBLE_DETECT   0b11000000  //  7     5                                                       sequencer off on preamble detect interrupt
#define SX127X_FROM_RX_TIMEOUT_RECEIVE                0b00000000  //  4     3     mode that will be set after Rx timeout: receive (default)
#define SX127X_FROM_RX_TIMEOUT_TRANSMIT               0b00001000  //  4     3                                             transmit
#define SX127X_FROM_RX_TIMEOUT_LP_SELECTION           0b00010000  //  4     3                                             low power selection
#define SX127X_FROM_RX_TIMEOUT_SEQ_OFF                0b00011000  //  4     3                                             sequencer off
#define SX127X_FROM_PACKET_RECEIVED_SEQ_OFF           0b00000000  //  2     0     mode that will be set after packet received: sequencer off (default)
#define SX127X_FROM_PACKET_RECEIVED_TRANSMIT          0b00000001  //  2     0                                                  transmit
#define SX127X_FROM_PACKET_RECEIVED_LP_SELECTION      0b00000010  //  2     0                                                  low power selection
#define SX127X_FROM_PACKET_RECEIVED_RECEIVE_FS        0b00000011  //  2     0                                                  receive via FS
#define SX127X_FROM_PACKET_RECEIVED_RECEIVE           0b00000100  //  2     0                                                  receive

// SX127X_REG_TIMER_RESOL
#define SX127X_TIMER1_OFF                             0b00000000  //  3     2     timer 1 resolution: disabled (default)
#define SX127X_TIMER1_RESOLUTION_64_US                0b00000100  //  3     2                         64 us
#define SX127X_TIMER1_RESOLUTION_4_1_MS               0b00001000  //  3     2                         4.1 ms
#define SX127X_TIMER1_RESOLUTION_262_MS               0b00001100  //  3     2                         262 ms
#define SX127X_TIMER2_OFF                             0b00000000  //  3     2     timer 2 resolution: disabled (default)
#define SX127X_TIMER2_RESOLUTION_64_US                0b00000001  //  3     2                         64 us
#define SX127X_TIMER2_RESOLUTION_4_1_MS               0b00000010  //  3     2                         4.1 ms
#define SX127X_TIMER2_RESOLUTION_262_MS               0b00000011  //  3     2                         262 ms

// SX127X_REG_TIMER1_COEF
#define SX127X_TIMER1_COEFFICIENT                     0xF5        //  7     0     multiplication coefficient for timer 1

// SX127X_REG_TIMER2_COEF
#define SX127X_TIMER2_COEFFICIENT                     0x20        //  7     0     multiplication coefficient for timer 2

// SX127X_REG_IMAGE_CAL
#define SX127X_AUTO_IMAGE_CAL_OFF                     0b00000000  //  7     7     temperature calibration disabled (default)
#define SX127X_AUTO_IMAGE_CAL_ON                      0b10000000  //  7     7     temperature calibration enabled
#define SX127X_IMAGE_CAL_START                        0b01000000  //  6     6     start temperature calibration
#define SX127X_IMAGE_CAL_RUNNING                      0b00100000  //  5     5     temperature calibration is on-going
#define SX127X_IMAGE_CAL_COMPLETE                     0b00000000  //  5     5     temperature calibration finished
#define SX127X_TEMP_CHANGED                           0b00001000  //  3     3     temperature changed more than TEMP_THRESHOLD since last calibration
#define SX127X_TEMP_THRESHOLD_5_DEG_C                 0b00000000  //  2     1     temperature change threshold: 5 deg. C
#define SX127X_TEMP_THRESHOLD_10_DEG_C                0b00000010  //  2     1                                   10 deg. C (default)
#define SX127X_TEMP_THRESHOLD_15_DEG_C                0b00000100  //  2     1                                   15 deg. C
#define SX127X_TEMP_THRESHOLD_20_DEG_C                0b00000110  //  2     1                                   20 deg. C
#define SX127X_TEMP_MONITOR_ON                        0b00000000  //  0     0     temperature monitoring enabled (default)
#define SX127X_TEMP_MONITOR_OFF                       0b00000001  //  0     0     temperature monitoring disabled

// SX127X_REG_LOW_BAT
#define SX127X_LOW_BAT_OFF                            0b00000000  //  3     3     low battery detector disabled
#define SX127X_LOW_BAT_ON                             0b00001000  //  3     3     low battery detector enabled
#define SX127X_LOW_BAT_TRIM_1_695_V                   0b00000000  //  2     0     battery voltage threshold: 1.695 V
#define SX127X_LOW_BAT_TRIM_1_764_V                   0b00000001  //  2     0                                1.764 V
#define SX127X_LOW_BAT_TRIM_1_835_V                   0b00000010  //  2     0                                1.835 V (default)
#define SX127X_LOW_BAT_TRIM_1_905_V                   0b00000011  //  2     0                                1.905 V
#define SX127X_LOW_BAT_TRIM_1_976_V                   0b00000100  //  2     0                                1.976 V
#define SX127X_LOW_BAT_TRIM_2_045_V                   0b00000101  //  2     0                                2.045 V
#define SX127X_LOW_BAT_TRIM_2_116_V                   0b00000110  //  2     0                                2.116 V
#define SX127X_LOW_BAT_TRIM_2_185_V                   0b00000111  //  2     0                                2.185 V

// SX127X_REG_IRQ_FLAGS_1
#define SX127X_FLAG_MODE_READY                        0b10000000  //  7     7     requested mode is ready
#define SX127X_FLAG_RX_READY                          0b01000000  //  6     6     reception ready (after RSSI, AGC, AFC)
#define SX127X_FLAG_TX_READY                          0b00100000  //  5     5     transmission ready (after PA ramp-up)
#define SX127X_FLAG_PLL_LOCK                          0b00010000  //  4     4     PLL locked
#define SX127X_FLAG_RSSI                              0b00001000  //  3     3     RSSI value exceeds RSSI threshold
#define SX127X_FLAG_TIMEOUT                           0b00000100  //  2     2     timeout occurred
#define SX127X_FLAG_PREAMBLE_DETECT                   0b00000010  //  1     1     valid preamble was detected
#define SX127X_FLAG_SYNC_ADDRESS_MATCH                0b00000001  //  0     0     sync address matched

// SX127X_REG_IRQ_FLAGS_2
#define SX127X_FLAG_FIFO_FULL                         0b10000000  //  7     7     FIFO is full
#define SX127X_FLAG_FIFO_EMPTY                        0b01000000  //  6     6     FIFO is empty
#define SX127X_FLAG_FIFO_LEVEL                        0b00100000  //  5     5     number of bytes in FIFO exceeds FIFO_THRESHOLD
#define SX127X_FLAG_FIFO_OVERRUN                      0b00010000  //  4     4     FIFO overrun occurred
#define SX127X_FLAG_PACKET_SENT                       0b00001000  //  3     3     packet was successfully sent
#define SX127X_FLAG_PAYLOAD_READY                     0b00000100  //  2     2     packet was successfully received
#define SX127X_FLAG_CRC_OK                            0b00000010  //  1     1     CRC check passed
#define SX127X_FLAG_LOW_BAT                           0b00000001  //  0     0     battery voltage dropped below threshold

// SX127X_REG_DIO_MAPPING_1
#define SX127X_DIO0_LORA_RX_DONE                      0b00000000  //  7     6
#define SX127X_DIO0_LORA_TX_DONE                      0b01000000  //  7     6
#define SX127X_DIO0_LORA_CAD_DONE                     0b10000000  //  7     6
#define SX127X_DIO0_CONT_MODE_READY                   0b11000000  //  7     6
#define SX127X_DIO0_CONT_SYNC_ADDRESS                 0b00000000  //  7     6
#define SX127X_DIO0_CONT_RSSI_PREAMBLE_DETECT         0b01000000  //  7     6
#define SX127X_DIO0_CONT_RX_READY                     0b10000000  //  7     6
#define SX127X_DIO0_CONT_TX_READY                     0b00000000  //  7     6
#define SX127X_DIO0_PACK_PAYLOAD_READY                0b00000000  //  7     6
#define SX127X_DIO0_PACK_PACKET_SENT                  0b00000000  //  7     6
#define SX127X_DIO0_PACK_CRC_OK                       0b01000000  //  7     6
#define SX127X_DIO0_PACK_TEMP_CHANGE_LOW_BAT          0b11000000  //  7     6
#define SX127X_DIO1_LORA_RX_TIMEOUT                   0b00000000  //  5     4
#define SX127X_DIO1_LORA_FHSS_CHANGE_CHANNEL          0b00010000  //  5     4
#define SX127X_DIO1_LORA_CAD_DETECTED                 0b00100000  //  5     4
#define SX127X_DIO1_CONT_DCLK                         0b00000000  //  5     4
#define SX127X_DIO1_CONT_RSSI_PREAMBLE_DETECT         0b00010000  //  5     4
#define SX127X_DIO1_PACK_FIFO_LEVEL                   0b00000000  //  5     4
#define SX127X_DIO1_PACK_FIFO_EMPTY                   0b00010000  //  5     4
#define SX127X_DIO1_PACK_FIFO_FULL                    0b00100000  //  5     4
#define SX127X_DIO2_LORA_FHSS_CHANGE_CHANNEL          0b00000000  //  3     2
#define SX127X_DIO2_CONT_DATA                         0b00000000  //  3     2
#define SX127X_DIO2_PACK_FIFO_FULL                    0b00000000  //  3     2
#define SX127X_DIO2_PACK_RX_READY                     0b00000100  //  3     2
#define SX127X_DIO2_PACK_TIMEOUT                      0b00001000  //  3     2
#define SX127X_DIO2_PACK_SYNC_ADDRESS                 0b00011000  //  3     2
#define SX127X_DIO3_LORA_CAD_DONE                     0b00000000  //  1     0
#define SX127X_DIO3_LORA_VALID_HEADER                 0b00000001  //  1     0
#define SX127X_DIO3_LORA_PAYLOAD_CRC_ERROR            0b00000010  //  1     0
#define SX127X_DIO3_CONT_TIMEOUT                      0b00000000  //  1     0
#define SX127X_DIO3_CONT_RSSI_PREAMBLE_DETECT         0b00000001  //  1     0
#define SX127X_DIO3_CONT_TEMP_CHANGE_LOW_BAT          0b00000011  //  1     0
#define SX127X_DIO3_PACK_FIFO_EMPTY                   0b00000000  //  1     0
#define SX127X_DIO3_PACK_TX_READY                     0b00000001  //  1     0

// SX127X_REG_DIO_MAPPING_2
#define SX127X_DIO4_LORA_CAD_DETECTED                 0b10000000  //  7     6
#define SX127X_DIO4_LORA_PLL_LOCK                     0b01000000  //  7     6
#define SX127X_DIO4_CONT_TEMP_CHANGE_LOW_BAT          0b00000000  //  7     6
#define SX127X_DIO4_CONT_PLL_LOCK                     0b01000000  //  7     6
#define SX127X_DIO4_CONT_TIMEOUT                      0b10000000  //  7     6
#define SX127X_DIO4_CONT_MODE_READY                   0b11000000  //  7     6
#define SX127X_DIO4_PACK_TEMP_CHANGE_LOW_BAT          0b00000000  //  7     6
#define SX127X_DIO4_PACK_PLL_LOCK                     0b01000000  //  7     6
#define SX127X_DIO4_PACK_TIMEOUT                      0b10000000  //  7     6
#define SX127X_DIO4_PACK_RSSI_PREAMBLE_DETECT         0b11000000  //  7     6
#define SX127X_DIO5_LORA_MODE_READY                   0b00000000  //  5     4
#define SX127X_DIO5_LORA_CLK_OUT                      0b00010000  //  5     4
#define SX127X_DIO5_CONT_CLK_OUT                      0b00000000  //  5     4
#define SX127X_DIO5_CONT_PLL_LOCK                     0b00010000  //  5     4
#define SX127X_DIO5_CONT_RSSI_PREAMBLE_DETECT         0b00100000  //  5     4
#define SX127X_DIO5_CONT_MODE_READY                   0b00110000  //  5     4
#define SX127X_DIO5_PACK_CLK_OUT                      0b00000000  //  5     4
#define SX127X_DIO5_PACK_PLL_LOCK                     0b00010000  //  5     4
#define SX127X_DIO5_PACK_DATA                         0b00100000  //  5     4
#define SX127X_DIO5_PACK_MODE_READY                   0b00110000  //  5     4
#define SX127X_DIO_MAP_PREAMBLE_DETECT                0b00000001  //  0     0
#define SX127X_DIO_MAP_RSSI                           0b00000000  //  0     0

// SX1272_REG_PLL_HOP + SX1278_REG_PLL_HOP
#define SX127X_FAST_HOP_OFF                           0b00000000  //  7     7     carrier frequency validated when FRF registers are written
#define SX127X_FAST_HOP_ON                            0b10000000  //  7     7     carrier frequency validated when FS modes are requested

// SX1272_REG_TCXO + SX1278_REG_TCXO
#define SX127X_TCXO_INPUT_EXTERNAL                    0b00000000  //  4     4     use external crystal oscillator
#define SX127X_TCXO_INPUT_EXTERNAL_CLIPPED            0b00010000  //  4     4     use external crystal oscillator clipped sine connected to XTA pin

// SX1272_REG_PLL + SX1278_REG_PLL
#define SX127X_PLL_BANDWIDTH_75_KHZ                   0b00000000  //  7     6     PLL bandwidth: 75 kHz
#define SX127X_PLL_BANDWIDTH_150_KHZ                  0b01000000  //  7     6                    150 kHz
#define SX127X_PLL_BANDWIDTH_225_KHZ                  0b10000000  //  7     6                    225 kHz
#define SX127X_PLL_BANDWIDTH_300_KHZ                  0b11000000  //  7     6                    300 kHz (default)

// SX1278 specific register map
#define SX1278_REG_MODEM_CONFIG_3                     0x26
#define SX1278_REG_PLL_HOP                            0x44
#define SX1278_REG_TCXO                               0x4B
#define SX1278_REG_PA_DAC                             0x4D
#define SX1278_REG_FORMER_TEMP                        0x5B
#define SX1278_REG_REG_BIT_RATE_FRAC                  0x5D
#define SX1278_REG_AGC_REF                            0x61
#define SX1278_REG_AGC_THRESH_1                       0x62
#define SX1278_REG_AGC_THRESH_2                       0x63
#define SX1278_REG_AGC_THRESH_3                       0x64
#define SX1278_REG_PLL                                0x70

// SX1278 LoRa modem settings
// SX1278_REG_OP_MODE                                                  MSB   LSB   DESCRIPTION
#define SX1278_HIGH_FREQ                              0b00000000  //  3     3     access HF test registers
#define SX1278_LOW_FREQ                               0b00001000  //  3     3     access LF test registers

// SX1278_REG_FRF_MSB + REG_FRF_MID + REG_FRF_LSB
#define SX1278_FRF_MSB                                0x6C        //  7     0     carrier frequency setting: f_RF = (F(XOSC) * FRF)/2^19
#define SX1278_FRF_MID                                0x80        //  7     0         where F(XOSC) = 32 MHz
#define SX1278_FRF_LSB                                0x00        //  7     0               FRF = 3 byte value of FRF registers

// SX1278_REG_PA_CONFIG
#define SX1278_MAX_POWER                              0b01110000  //  6     4     max power: P_max = 10.8 + 0.6*MAX_POWER [dBm]; P_max(MAX_POWER = 0b111) = 15 dBm
#define SX1278_LOW_POWER                              0b00100000  //  6     4

// SX1278_REG_LNA
#define SX1278_LNA_BOOST_LF_OFF                       0b00000000  //  4     3     default LNA current

// SX127X_REG_MODEM_CONFIG_1
#define SX1278_BW_7_80_KHZ                            0b00000000  //  7     4     bandwidth:  7.80 kHz
#define SX1278_BW_10_40_KHZ                           0b00010000  //  7     4                 10.40 kHz
#define SX1278_BW_15_60_KHZ                           0b00100000  //  7     4                 15.60 kHz
#define SX1278_BW_20_80_KHZ                           0b00110000  //  7     4                 20.80 kHz
#define SX1278_BW_31_25_KHZ                           0b01000000  //  7     4                 31.25 kHz
#define SX1278_BW_41_70_KHZ                           0b01010000  //  7     4                 41.70 kHz
#define SX1278_BW_62_50_KHZ                           0b01100000  //  7     4                 62.50 kHz
#define SX1278_BW_125_00_KHZ                          0b01110000  //  7     4                 125.00 kHz
#define SX1278_BW_250_00_KHZ                          0b10000000  //  7     4                 250.00 kHz
#define SX1278_BW_500_00_KHZ                          0b10010000  //  7     4                 500.00 kHz
#define SX1278_CR_4_5                                 0b00000010  //  3     1     error coding rate:  4/5
#define SX1278_CR_4_6                                 0b00000100  //  3     1                         4/6
#define SX1278_CR_4_7                                 0b00000110  //  3     1                         4/7
#define SX1278_CR_4_8                                 0b00001000  //  3     1                         4/8
#define SX1278_HEADER_EXPL_MODE                       0b00000000  //  0     0     explicit header mode
#define SX1278_HEADER_IMPL_MODE                       0b00000001  //  0     0     implicit header mode

// SX127X_REG_MODEM_CONFIG_2
#define SX1278_RX_CRC_MODE_OFF                        0b00000000  //  2     2     CRC disabled
#define SX1278_RX_CRC_MODE_ON                         0b00000100  //  2     2     CRC enabled

// SX1278_REG_MODEM_CONFIG_3
#define SX1278_LOW_DATA_RATE_OPT_OFF                  0b00000000  //  3     3     low data rate optimization disabled
#define SX1278_LOW_DATA_RATE_OPT_ON                   0b00001000  //  3     3     low data rate optimization enabled
#define SX1278_AGC_AUTO_OFF                           0b00000000  //  2     2     LNA gain set by REG_LNA
#define SX1278_AGC_AUTO_ON                            0b00000100  //  2     2     LNA gain set by internal AGC loop

// SX127X_REG_VERSION
#define SX1278_CHIP_VERSION                           0x12

// SX1278 FSK modem settings
// SX127X_REG_PA_RAMP
#define SX1278_NO_SHAPING                             0b00000000  //  6     5     data shaping: no shaping (default)
#define SX1278_FSK_GAUSSIAN_1_0                       0b00100000  //  6     5                   FSK modulation Gaussian filter, BT = 1.0
#define SX1278_FSK_GAUSSIAN_0_5                       0b01000000  //  6     5                   FSK modulation Gaussian filter, BT = 0.5
#define SX1278_FSK_GAUSSIAN_0_3                       0b01100000  //  6     5                   FSK modulation Gaussian filter, BT = 0.3
#define SX1278_OOK_FILTER_BR                          0b00100000  //  6     5                   OOK modulation filter, f_cutoff = BR
#define SX1278_OOK_FILTER_2BR                         0b01000000  //  6     5                   OOK modulation filter, f_cutoff = 2*BR

// SX1278_REG_AGC_REF
#define SX1278_AGC_REFERENCE_LEVEL_LF                 0x19        //  5     0     floor reference for AGC thresholds: AgcRef = -174 + 10*log(2*RxBw) + 8 + AGC_REFERENCE_LEVEL [dBm]: below 525 MHz
#define SX1278_AGC_REFERENCE_LEVEL_HF                 0x1C        //  5     0                                                                                                         above 779 MHz

// SX1278_REG_AGC_THRESH_1
#define SX1278_AGC_STEP_1_LF                          0x0C        //  4     0     1st AGC threshold: below 525 MHz
#define SX1278_AGC_STEP_1_HF                          0x0E        //  4     0                        above 779 MHz

// SX1278_REG_AGC_THRESH_2
#define SX1278_AGC_STEP_2_LF                          0x40        //  7     4     2nd AGC threshold: below 525 MHz
#define SX1278_AGC_STEP_2_HF                          0x50        //  7     4                        above 779 MHz
#define SX1278_AGC_STEP_3                             0x0B        //  3     0     3rd AGC threshold

// SX1278_REG_AGC_THRESH_3
#define SX1278_AGC_STEP_4                             0xC0        //  7     4     4th AGC threshold
#define SX1278_AGC_STEP_5                             0x0C        //  4     0     5th AGC threshold

/***************************************************************************/
/*                                Types                                    */
/***************************************************************************/

/***************************************************************************/
/*                               Classes                                   */
/***************************************************************************/

#endif
