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

#pragma once

#include <app/util/basic-types.h>

/** @brief Start Feedback.
 *
 * This function is called by the Identify plugin when identification begins.
 * It informs the Identify Feedback plugin that it should begin providing its
 * implemented feedback functionality (e.g. LED blinking, buzzer sounding, etc.)
 * until the Identify plugin tells it to stop.
 * The identify time is purely a matter of informational convenience; this plugin
 * does not need to know how long it will identify (the Identify plugin will
 * perform the necessary timekeeping.)
 *
 * @param endpoint The endpoint.  Ver.: always
 * @param identityTime  Ver.: always
 */
bool emberAfPluginIdentifyStartFeedbackCallback(chip::EndpointId endpoint, uint16_t identifyTime);

/** @brief Stop Feedback.
 *
 * This function is called by the Identify plugin when identification is finished.
 * It tells the Identify Feedback plugin to stop providing its implemented feedback
 * functionality.
 *
 * @param endpoint The endpoint.  Ver.: always
 */
bool emberAfPluginIdentifyStopFeedbackCallback(chip::EndpointId endpoint);
