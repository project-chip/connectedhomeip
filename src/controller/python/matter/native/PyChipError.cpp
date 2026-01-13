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

#include "PyChipError.h"

#include <cstring>

#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::DeviceLayer;

void pychip_FormatError(PyChipError * apError, char * apBuf, uint32_t aBufSize)
{
    // Note: ChipError::AsString is not thread safe since it accesses a global (and mutable) variable for holding the formatted
    // string.
    // TODO: We should consider providing an API for putting the string into a user provided buffer to avoid potential race
    // conditions.
#if CHIP_STACK_LOCK_TRACKING_ENABLED
    if (!PlatformMgr().IsChipStackLockedByCurrentThread())
    {
        PlatformMgr().LockChipStack();
        snprintf(apBuf, aBufSize, "%s", CHIP_ERROR(apError->mCode, apError->mFile, apError->mLine).AsString());
        PlatformMgr().UnlockChipStack();
    }
    else
#endif
    {
        snprintf(apBuf, aBufSize, "%s", CHIP_ERROR(apError->mCode, apError->mFile, apError->mLine).AsString());
    }
}
