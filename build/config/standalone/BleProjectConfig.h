/*
 *
 *    Copyright (c) 2017 Nest Labs, Inc.
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
/**
 *    @file
 *      BLE project configuration for standalone builds on Linux and OS X.
 *
 */
#ifndef BLEPROJECTCONFIG_H
#define BLEPROJECTCONFIG_H

// Set the number of BLEEndPoints to a number greater than one (1) since
// there can be a delay between closing the endpoint and when the end point
// is actually closed and the end point resources are available for reuse.
//
// Experimentation has shown that four (4) tends to be a reasonable number.
#define BLE_LAYER_NUM_BLE_ENDPOINTS 4

#endif /* BLEPROJECTCONFIG_H */
