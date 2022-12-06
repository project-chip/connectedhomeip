/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#pragma once

#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION 0
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP 0

// Use a default pairing code if one hasn't been provisioned in flash.
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE 20202021
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR 0xF00

// Lock Manager settings
#define CHIP_LOCK_MANAGER_USER_NUMBER 10
#define CHIP_LOCK_MANAGER_CREDENTIALS_NUMBER 10
#define CHIP_LOCK_MANAGER_CREDENTIALS_PER_USER_NUMBER 5
#define CHIP_LOCK_MANAGER_WEEK_DAY_SCHEDULES_PER_USER_NUMBER 10
#define CHIP_LOCK_MANAGER_YEAR_DAY_SCHEDULES_PER_USER_NUMBER 10
