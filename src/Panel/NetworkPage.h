/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Handler of the Network page                                   *
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

#ifndef NETWORKPAGE_H_
#define NETWORKPAGE_H_

/***************************************************************************/
/*                              Includes                                   */
/***************************************************************************/

#include "MicronetDevice.h"
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

class NetworkPage : public PageHandler
{
  public:
    NetworkPage();
    virtual ~NetworkPage();

    void Draw(bool force);
    void SetNetworkStatus(DeviceInfo_t &deviceInfo);

  private:
    bool             networkConnected;
    uint32_t         deviceId;
    NetworkMap_t     networkMap;
    uint32_t         nbDevicesInRange;
    ConnectionInfo_t devicesInRange[MAX_DEVICES_PER_NETWORK];

    void                 DrawDeviceIcon(uint8_t const *icon, uint32_t position, uint32_t radioLevel);
    unsigned char const *GetIconById(uint32_t deviceId);
};

#endif
