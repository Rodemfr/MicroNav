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
#include "InfoPageCompass.h"
#include "InfoPageMicronet.h"
#include "InfoPagePower.h"
#include "InfoPageSensors.h"
#include "LogoPage.h"
#include "MicronetDevice.h"
#include "NavigationData.h"
#include "NetworkPage.h"
#include "PageHandler.h"
#include "TopicHandler.h"

#include <Arduino.h>
#include <vector>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

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
    void DrawPage();
    void DrawPageISR();
    void NextTopic();
    void NextTopicISR();
    void SetNavigationData(NavigationData *navData);
    void SetNetworkStatus(DeviceInfo_t &networkStatus);
    void LowPower(bool enable);

  private:
    bool                        displayAvailable;
    uint32_t                    topicIndex;
    TopicHandler               *currentTopic;
    std::vector<TopicHandler *> topicList;
    TopicHandler                statusTopic;
    TopicHandler                infoTopic;
    TopicHandler                configTopic;
    TopicHandler                commandTopic;

    LogoPage         logoPage;
    ClockPage        clockPage;
    NetworkPage      networkPage;
    InfoPageMicronet infoPageMicronet;
    InfoPageSensors  infoPageSensors;
    InfoPagePower    infoPagePower;
    InfoPageCompass  infoPageCompass;
    ConfigPage1      configPage1;
    ConfigPage2      configPage2;
    CommandPage      commandPage;

    NavigationData    *navData;
    DeviceInfo_t       networkStatus;
    uint32_t           lastRelease   = 0;
    uint32_t           lastPress     = 0;
    bool               buttonPressed = false;
    portMUX_TYPE       commandMutex;
    portMUX_TYPE       buttonMutex;
    TaskHandle_t       commandTaskHandle;
    EventGroupHandle_t commandEventGroup;

    static PanelManager *objectPtr;
    static void          CommandProcessingTask(void *callingObject);
    static void          StaticButtonIsr();
    void                 ButtonIsr();
    static void          PowerButtonCallback(bool longPress);

    void CommandCallback();
};

#endif
