/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

namespace chip {
namespace Shell {

/**
 * This function registers the base64 encode/decode commands.
 *
 */
void CommandBase64Init();

/**
 * This function registers the BLE commands.
 *
 */
void CommandBLEInit();

/**
 * This function registers the common commands.
 *
 */
void CommandCommonInit();

/**
 * This function registers the device configuration commands.
 *
 */
void CommandConfigInit();

/**
 * This function registers the wifi commands.
 *
 */
void CommandWifiInit();

} // namespace Shell
} // namespace chip
