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
 * @brief Definitions for the Bulb UI plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_BULB_UI_H
#define SILABS_BULB_UI_H

/** @brief Starts the search for joinable networks.
 *
 * Starts the bulb-UI search for joinable networks.  This
 * is not normally required, as the bulb-UI will do this automatically.
 * However, some plugins that interrupt the normal bulb-UI behavior,
 * such as the manufacturing library cluster server, may need to kick off
 * a network search when their task is complete.
 *
 */
void emberAfPluginBulbUiInitiateNetworkSearch(void);

#endif
