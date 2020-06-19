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
 * @brief APIs and defines for the DMP UI Stub plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef DMP_UI_STUB_H
#define DMP_UI_STUB_H

// Stubs
#define dmpUiDisplayZigBeeState(arg)
#define dmpUiZigBeePjoin(arg)
#define dmpUiLightOff()
#define dmpUiLightOn()
#define dmpUiUpdateDirection(arg)
#define dmpUiBluetoothConnected(arg)
#define dmpUiSetBleDeviceName(arg)
#define dmpUiInit()
#define dmpUiDisplayHelp()
#define DMP_UI_DIRECTION_INVALID 0
#define DMP_UI_DIRECTION_BLUETOOTH 1
#define DMP_UI_DIRECTION_SWITCH 2
#define DMP_UI_DIRECTION_ZIGBEE  3
#define DmpUiLightDirection_t  uint8_t

#endif //DMP_UI_STUB_H
