/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
