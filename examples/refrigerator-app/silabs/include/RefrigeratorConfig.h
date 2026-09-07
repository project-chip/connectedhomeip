/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
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

#ifndef SL_MATTER_REFRIGERATOR_CONFIG_H
#define SL_MATTER_REFRIGERATOR_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// RefrigeratorConfig.h
// Preprocessor knobs for the silabs refrigerator example. Edit these #define
// statements to customize the endpoint composition of the device. The
// refrigerator endpoint is the tree-composition root, the cold and freeze
// cabinet endpoints are parented to it.

// <o REFRIGERATOR_ENDPOINT> Refrigerator endpoint
// <i> Default: 1
#define REFRIGERATOR_ENDPOINT 1

// <o COLD_CABINET_ENDPOINT> Temperature controlled cold cabinet endpoint
// <i> Default: 2
#define COLD_CABINET_ENDPOINT 2

// <o FREEZE_CABINET_ENDPOINT> Temperature controlled freeze cabinet endpoint
// <i> Default: 3
#define FREEZE_CABINET_ENDPOINT 3

// <<< end of configuration section >>>

#endif // SL_MATTER_REFRIGERATOR_CONFIG_H
