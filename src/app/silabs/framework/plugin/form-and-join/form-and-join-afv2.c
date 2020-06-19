/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 * @brief This provides a compatibility layer between the legacy form and join code
 * and the App Framework v2 code.  The legacy form and join code is agnostic
 * of the Application Framework and thus we cannot put lots of App Framework
 * specific code in there.
 * More specifically, the sensor/sink app doesn't use the framework and thus
 * cannot compile form-and-join code with Afv2 references in it.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/common/form-and-join.h"

// ****************************************************************************
// Globals

EmberEventControl emberAfPluginFormAndJoinCleanupEventControl;

// ****************************************************************************
// Forward Declarations

// ****************************************************************************

void emberAfUnusedPanIdFoundHandler(EmberPanId panId, uint8_t channel)
{
  // It is not necessary to wrap these in push/pop calls because they
  // are executing in the context of a stack callback, which already has done
  // a push.
  emberAfPluginFormAndJoinUnusedPanIdFoundCallback(panId, channel);
}

void emberJoinableNetworkFoundHandler(EmberZigbeeNetwork *networkFound,
                                      uint8_t lqi,
                                      int8_t rssi)
{
  emberAfPluginFormAndJoinNetworkFoundCallback(networkFound, lqi, rssi);
}

void emberAfPluginFormAndJoinCleanupEventHandler(void)
{
  // This takes a bit of explaining.
  // Prior to this release the form-and-join library was not a plugin and was
  // hardcoded in the project templates, thus there was no way to turn it off.
  // We needed to be able to enable/disable it and so we made it into a plugin.

  // The Network Find was another plugin layered on top of the form-and-join library.
  // Any application that used form-and-join without the network-find plugin
  // would need to properly cleanup the form-and-join code.  On the SOC this
  // was done automatically by a timer that fired after 30 seconds, assuming
  // you called emberFormAndJoinTick() regularly.  On the host, there was nno
  // timer and thus an explicit call to emberFormAndJoinCleanup() was required.

  // Now if the network-find plugin was enabled it had its own cleanup
  // (that also called emberFormAndJoinCleanup()), therefore it was
  // unnecessary to schedule an event.

  // To maintain backwards compatibility we will run the cleanup code
  // only on SOC when it is not disabled by another piece of code
  // (e.g. network-find plugin)
#if !defined(EMBER_AF_DISABLE_FORM_AND_JOIN_TICK) && !defined(EZSP_HOST)
  emberFormAndJoinCleanup(EMBER_SUCCESS);
#else
  // This is a workaround put in place to handle bug EMAPPFWKV2-1379.  There are
  // certain circumstances in which the CleanupEvent is set active while the
  // above #if conditions are not met.  This results in the eventHandler taking
  // no action, leaving the event in an active state.  The event then has no
  // means by which it can be made inactive, so the scheduler indefinitely
  // calls the empty event, which prevents the device from sleeping.
  emberEventControlSetInactive(emberAfPluginFormAndJoinCleanupEventControl);
#endif
}
