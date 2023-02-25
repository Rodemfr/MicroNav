/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Driver for T-BEAM 1.1 OLED Panel and button                   *
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

#ifndef PANELDRIVER_H_
#define PANELDRIVER_H_

/***************************************************************************/
/*                              Includes                                   */
/***************************************************************************/

#include "ClockPage.h"
#include "CommandPage.h"
#include "ConfigPage1.h"
#include "ConfigPage2.h"
#include "InfoPage.h"
#include "LogoPage.h"
#include "MicronetDevice.h"
#include "NavigationData.h"
#include "NetworkPage.h"
#include "PageHandler.h"


#include <Arduino.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

enum
{
    PAGE_LOGO = 0,
    PAGE_CLOCK,
    PAGE_NETWORK,
    PAGE_INFO,
    PAGE_CONFIG1,
    PAGE_CONFIG2,
    PAGE_COMMAND,
    PAGE_MAX_PAGES
} PanelPages_t;

/***************************************************************************/
/*                                Types                                    */
/***************************************************************************/

/***************************************************************************/
/*                               Classes                                   */
/***************************************************************************/

class PanelManager
{
  public:
    PanelManager();
    ~PanelManager();

    bool Init();
    void SetPage(uint32_t pageNumber);
    void SetPageISR(uint32_t pageNumber);
    void DrawPage();
    void DrawPageISR();
    void NextPage();
    void NextPageISR();
    void SetNavigationData(NavigationData *navData);
    void SetNetworkStatus(DeviceInfo_t &networkStatus);
    void LowPower(bool enable);

  private:
    bool               displayAvailable;
    uint32_t           pageNumber;
    PageHandler       *currentPage;
    TaskHandle_t       commandTaskHandle;
    EventGroupHandle_t commandEventGroup;
    LogoPage           logoPage;
    ClockPage          clockPage;
    NetworkPage        networkPage;
    InfoPage           infoPage;
    ConfigPage1        configPage1;
    ConfigPage2        configPage2;
    CommandPage        commandPage;
    portMUX_TYPE       commandMutex;
    portMUX_TYPE       buttonMutex;
    NavigationData    *navData;
    DeviceInfo_t       networkStatus;
    uint32_t           lastRelease   = 0;
    uint32_t           lastPress     = 0;
    bool               buttonPressed = false;

    static PanelManager *objectPtr;
    static void          CommandProcessingTask(void *callingObject);
    static void          StaticButtonIsr();
    void                 ButtonIsr();

    void CommandCallback();
};

#endif
