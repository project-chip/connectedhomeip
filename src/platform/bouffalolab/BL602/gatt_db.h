/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Google LLC.
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

#ifndef __GATT_DB_H
#define __GATT_DB_H

#if __cplusplus
extern "C" {
#endif

#include "bg_gattdb_def.h"

extern const struct bg_gattdb_def bg_gattdb_data;

#define gattdb_service_changed_char 3
#define gattdb_database_hash 6
#define gattdb_client_support_features 8
#define gattdb_device_name 11
#define gattdb_ota_control 23
#define gattdb_CHIPoBLEChar_Rx 26
#define gattdb_CHIPoBLEChar_Tx 28

#if __cplusplus
}
#endif

#endif
