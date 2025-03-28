/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void uartInit(void);
int16_t uartWrite(const char * Buf, uint16_t BufLength);
int16_t uartRead(char * Buf, uint16_t NbBytesToRead);

#ifdef CFG_USB_CDC_ENABLE
void aosUartRxCallback(int fd, void * param);
#endif

#ifdef __cplusplus
} // extern "C"
#endif
