/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: (c) 2020 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
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
