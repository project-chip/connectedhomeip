/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#ifndef SL_MATTER_LOCK_CONFIG_H
#define SL_MATTER_LOCK_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// LockConfig.h
// Preprocessor knobs for the silabs lock example. Edit these #define
// statements to customize the door lock cluster endpoint and actuator-related
// timer periods. Values must stay consistent with the Matter data model (ZAP)
// for the chosen endpoint.

// <o LOCK_ENDPOINT> Door Lock cluster endpoint
// <i> Default: 1
#define LOCK_ENDPOINT 1

// <o ACTUATOR_MOVEMENT_PERIOD_MS> Simulated actuator transition time in milliseconds
// <i> One-shot timer duration from initiating lock, unlock, or unlatch until movement completes.
// <i> Default: 10
#define ACTUATOR_MOVEMENT_PERIOD_MS 10

// <o UNLATCH_TIME_MS> Time in the unlatched state before transitioning to unlocked (ms)
// <i> Default: 100 (short interval for demo/testing)
#define UNLATCH_TIME_MS 100

// <<< end of configuration section >>>

#endif // SL_MATTER_LOCK_CONFIG_H
