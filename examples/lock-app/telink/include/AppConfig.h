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

#pragma once

// ---- Lock Example App Config ----

#define LOCK_MANAGER_CONFIG_USE_NVM_CREDENTIAL_STORAGE 1
#define LOCK_MANAGER_ACTUATOR_MOVEMENT_TIME_MS 2000

#define APP_MAX_USERS 10
#define APP_MAX_CREDENTIAL 10
#define APP_MAX_WEEKDAY_SCHEDULE_PER_USER 10
#define APP_MAX_YEARDAY_SCHEDULE_PER_USER 10
#define APP_MAX_HOLYDAY_SCHEDULE_PER_USER 10

#define APP_MAX_SCHEDULES_TOTAL 100

#define APP_DEFAULT_USERS_COUNT 5
#define APP_DEFAULT_CREDENTIAL_COUNT 5
#define APP_DEFAULT_WEEKDAY_SCHEDULE_PER_USER_COUNT 5
#define APP_DEFAULT_YEARDAY_SCHEDULE_PER_USER_COUNT 5
#define APP_DEFAULT_HOLYDAY_SCHEDULE_PER_USER_COUNT 5

#define APP_SET_DEVICE_INFO_PROVIDER 1
#define APP_SET_NETWORK_COMM_ENDPOINT_SEC 0
