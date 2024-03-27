/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
 *          Example project configuration file for CHIP.
 *
 *          This is a place to put application or project-specific overrides
 *          to the default configuration values for general CHIP features.
 *
 */

#pragma once

/**
 * @def CONFIG_BUILD_FOR_HOST_UNIT_TEST
 *
 * @brief Defines whether we're currently building for unit testing, which enables a set of features
 *        that are only utilized in those tests. This flag should not be enabled on devices. If you have a test
 *        that uses this flag, either appropriately conditionalize the entire test on this flag, or to exclude
 *        the compliation of that test source file entirely.
 */
#define CONFIG_BUILD_FOR_HOST_UNIT_TEST 1
