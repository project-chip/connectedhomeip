/* USER CODE BEGIN Header */
/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License,
 * Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy
 * of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to
 * in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *
 * limitations under the License.
 */
/* USER CODE END Header */

#ifndef BPKA_H__
#define BPKA_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
/* Enumerated values used for the return of the functions:
 * (warning: this enum must be aligned with BLEPLAT corresponding one)
 */
enum
{
    BPKA_OK    = 0,
    BPKA_FULL  = -1,
    BPKA_BUSY  = -2,
    BPKA_EOF   = -3,
    BPKA_ERROR = -5
};

void BPKA_Reset(void);

int BPKA_StartP256Key(const uint32_t * local_private_key);

void BPKA_ReadP256Key(uint32_t * local_public_key);

int BPKA_StartDhKey(const uint32_t * local_private_key, const uint32_t * remote_public_key);

int BPKA_ReadDhKey(uint32_t * dh_key);

int BPKA_Process(void);

void BPKA_BG_Process(void);

/* Callback used by BPKA_Process to indicate the end of the processing
 */
void BPKACB_Complete(void);

void BPKACB_Process(void);

#ifdef __cplusplus
}
#endif

#endif /* BPKA_H__ */
