/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Handler of the Config page                                      *
 * Author:   Ronan Demoment                                                *
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

#ifndef CONFIGPAGE1_H_
#define CONFIGPAGE1_H_

/***************************************************************************/
/*                              Includes                                   */
/***************************************************************************/

#include "PageHandler.h"

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

/***************************************************************************/
/*                                Types                                    */
/***************************************************************************/

/***************************************************************************/
/*                               Classes                                   */
/***************************************************************************/

class ConfigPage1 : public PageHandler
{
  public:
    ConfigPage1();
    virtual ~ConfigPage1();

    void Draw(bool force);
    PageAction_t OnButtonPressed(bool longPress);

  private:
    uint8_t swMajorVersion, swMinorVersion;
    uint32_t swPatchVersion;
    bool editMode;
    uint32_t editPosition;

    uint32_t configFreqSel;
    uint32_t configNmeaSel;
    bool configRmbWorkaround;
    bool configWindRepeater;

    void DeployConfiguration();

    char const *ConfigString(uint32_t index);
    char const *ConfigFreqString();
    char const *ConfigNmeaString();
    char const *ConfigRmbWorkaroundString();
    char const *ConfigWindRepeaterString();

    void ConfigCycle(uint32_t index);
    void ConfigFreqCycle();
    void ConfigNmeaCycle();
    void ConfigRmbWorkaroundCycle();
    void ConfigWindRepeaterCycle();
};

#endif
