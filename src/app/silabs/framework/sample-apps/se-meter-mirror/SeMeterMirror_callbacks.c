/**
 *
 *    Copyright (c) 2020 Silicon Labs
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
   ******************************************************************************/

// *******************************************************************
// * SeMeterMirror_callbacks.c
// *
// * This file contains all application specific code for the
// * meter mirror sample application. The code is implemented in
// * a series of callbacks which are called from within the
// * application framework. Each callback below includes a
// * comment explaining what it does for the application.
// *
// * This application is inteded to be used along with the
// * gas meter sample application SeMeterGas
// *
// * This application is further explained in the README.txt file which
// * accompanies it in the sample application directory here:
// * /app/framework/sample-apps/se-meter-mirror/README.xml
// *
// * It should also be noted that this application uses some
// * manufacturer specific attributes created by Ember. For instance
// * there was (as of this writing) no SE Mirror server attribute to
// * store the IEEE address of a mirrored device. Noticing this absence
// * we added a manufacturer specific IEEE address. You will notice
// * that every API which deals with the IEEE address of the device
// * being mirrored needs to include the Ember manufacturer code
// * along with it so that it can find the manufacturer specific
// * attribute.
// *
// * The use of a manufacturer specific attribute for IEEE address allows
// * the application to scale better, in the event that you wish to add more
// * mirrors and als to take advantage of the data persistence attribute storage in
// * the application framework.
// *
// * Copyright 2007 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#include "app/framework/util/common.h"
#include "app/framework/util/attribute-table.h"

/** @brief Finished
 *
 * This callback is fired when the network-find plugin is finished with the
 * forming or joining process.  The result of the operation will be returned
 * in the status parameter.
 *
 * @param status   Ver.: always
 */
void emberAfPluginNetworkFindFinishedCallback(EmberStatus status)
{
}

/** @brief Join
 *
 * This callback is called by the plugin when a joinable network has been
 * found.  If the application returns true, the plugin will attempt to join
 * the network.  Otherwise, the plugin will ignore the network and continue
 * searching.  Applications can use this callback to implement a network
 * blacklist.
 *
 * @param networkFound   Ver.: always
 * @param lqi   Ver.: always
 * @param rssi   Ver.: always
 */
bool emberAfPluginNetworkFindJoinCallback(EmberZigbeeNetwork *networkFound,
                                          uint8_t lqi,
                                          int8_t rssi)
{
  return true;
}

/** @brief Select File Descriptors
 *
 * This function is called when the Gateway plugin will do a select() call to
 * yield the processor until it has a timed event that needs to execute.  The
 * function implementor may add additional file descriptors that the
 * application will monitor with select() for data ready.  These file
 * descriptors must be read file descriptors.  The number of file descriptors
 * added must be returned by the function (0 for none added).
 *
 * @param list A pointer to a list of File descriptors that the function
 * implementor may append to  Ver.: always
 * @param maxSize The maximum number of elements that the function implementor
 * may add.  Ver.: always
 */
int emberAfPluginGatewaySelectFileDescriptorsCallback(int* list,
                                                      int maxSize)
{
  return 0;
}

/** @brief Broadcast Sent
 *
 * This function is called when a new MTORR broadcast has been successfully
 * sent.
 *
 */
void emberAfPluginConcentratorBroadcastSentCallback(void)
{
}

/** @brief Button Event
 *
 * This allows another module to get notification when a button is pressed and
 * released but the button joining plugin did not handle it.  This callback is
 * NOT called in ISR context so there are no restrictions on what code can
 * execute.
 *
 * @param buttonNumber The button number that was pressed.  Ver.: always
 * @param buttonPressDurationMs The length of time button was held down before
 * it was released.  Ver.: always
 */
void emberAfPluginButtonJoiningButtonEventCallback(uint8_t buttonNumber,
                                                   uint32_t buttonPressDurationMs)
{
}
