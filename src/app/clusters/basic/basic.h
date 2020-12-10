/*
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

/** @brief Basic Cluster Reset To Factory Defaults
 *
 * This function is called by the Basic server plugin when a request to
 * reset to factory defaults is received. The plugin will reset attributes
 * managed by the framework to their default values.
 * The application should perform any other necessary reset-related operations
 * in this callback, including resetting any externally-stored attributes.
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 */
void emberAfPluginBasicResetToFactoryDefaultsCallback(chip::EndpointId endpoint);
