/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Navigation data router                                        *
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

#ifndef NMEABRIDGE_H_
#define NMEABRIDGE_H_

/***************************************************************************/
/*                              Includes                                   */
/***************************************************************************/

#include "Configuration.h"
#include "MicronetCodec.h"
#include "NavigationData.h"

#include <stdint.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

#define NMEA_SENTENCE_MAX_LENGTH 128
#define NMEA_SENTENCE_HISTORY_SIZE 24

/***************************************************************************/
/*                                Types                                    */
/***************************************************************************/

typedef enum {
  NMEA_ID_UNKNOWN,
  NMEA_ID_RMB,
  NMEA_ID_RMC,
  NMEA_ID_GGA,
  NMEA_ID_VTG,
  NMEA_ID_MWV,
  NMEA_ID_DPT,
  NMEA_ID_VHW,
  NMEA_ID_HDG
} NmeaId_t;

typedef struct {
  uint32_t vwr;
  uint32_t vwt;
  uint32_t dpt;
  uint32_t mtw;
  uint32_t vlw;
  uint32_t vhw;
  uint32_t hdg;
  uint32_t vcc;
} NmeaTimeStamps_t;

#define NMEA_SENTENCE_MIN_PERIOD_MS 500

class NmeaBridge {
public:
  NmeaBridge(MicronetCodec *micronetCodec);
  virtual ~NmeaBridge();

  void PushNmeaChar(char c, LinkId_t sourceLink);
  void UpdateCompassData(float heading_deg);
  void UpdateMicronetData();

private:
  static const uint8_t asciiTable[128];
  char nmeaExtBuffer[NMEA_SENTENCE_MAX_LENGTH];
  char nmeaGnssBuffer[NMEA_SENTENCE_MAX_LENGTH];
  int nmeaExtWriteIndex;
  int nmeaGnssWriteIndex;
  NmeaTimeStamps_t nmeaTimeStamps;
  MicronetCodec *micronetCodec;

  bool IsSentenceValid(char *nmeaBuffer);
  NmeaId_t SentenceId(char *nmeaBuffer);
  void DecodeRMBSentence(char *sentence);
  void DecodeRMCSentence(char *sentence);
  void DecodeGGASentence(char *sentence);
  void DecodeVTGSentence(char *sentence);
  void DecodeMWVSentence(char *sentence);
  void DecodeDPTSentence(char *sentence);
  void DecodeVHWSentence(char *sentence);
  void DecodeHDGSentence(char *sentence);
  int16_t NibbleValue(char c);

  void EncodeMWV_R();
  void EncodeMWV_T();
  void EncodeDPT();
  void EncodeMTW();
  void EncodeVLW();
  void EncodeVHW();
  void EncodeHDG();
  void EncodeXDR();

  uint8_t AddNmeaChecksum(char *sentence);
};

/***************************************************************************/
/*                              Prototypes                                 */
/***************************************************************************/

#endif /* NMEABRIDGE_H_ */
