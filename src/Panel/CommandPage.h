/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Handler of the Command page                                   *
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

#ifndef COMMANDPAGE_H_
#define COMMANDPAGE_H_

/***************************************************************************/
/*                              Includes                                   */
/***************************************************************************/

#include "AttachPage.h"
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

class CommandPage : public PageHandler
{
  public:
    CommandPage();
    virtual ~CommandPage();

    virtual void SetDisplay(Adafruit_SSD1306 *display);
    void         Draw(bool force);
    PageAction_t OnButtonPressed(bool longPress);
    void         SetNetworkStatus(DeviceInfo_t &deviceInfo);

  private:
    PageHandler  *subPage;
    bool          editMode;
    uint32_t      editPosition;

    AttachPage attachPage;
};

#endif
