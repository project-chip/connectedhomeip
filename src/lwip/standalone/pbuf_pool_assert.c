/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "lwip/pbuf.h"
#include "lwipopts.h"

/**
 * PBUF_POOL_BUFSIZE, defined in lwipopts.h was previously using sizeof, but as it is used in other preprocessor macros in lwip
 * code, it failed. Below is to make sure that current calculation aligns with previous.
 */
_Static_assert((LWIP_MEM_ALIGN_SIZE(2 * __SIZEOF_POINTER__ + (2 * 2) + (4 * 1) + 1) ==
                LWIP_MEM_ALIGN_SIZE(sizeof(struct pbuf) + 1)),
               "PBUF_POOL_BUFFSIZE in lwipopts.h needs adjusting.");
