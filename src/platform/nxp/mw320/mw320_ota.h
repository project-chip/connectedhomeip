/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

typedef void * fw_update_id_t;

fw_update_id_t mw320_fw_update_begin(void);
int mw320_fw_update_wrblock(fw_update_id_t fwup_id, unsigned char * pblock, unsigned int blksize);
int mw320_fw_update_end(fw_update_id_t fwup_id, int rst_delay_sec);
int mw320_fw_update_abort(fw_update_id_t fwup_id);

#define MW320_OTA_TEST 0
#if (MW320_OTA_TEST == 1)
// Module Test function
void mw320_fw_update_test(void);
#endif // MW320_OTA_TEST
