/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
