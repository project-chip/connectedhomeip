/**
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <Matter/MTRDefines.h>

#if MTR_PER_CONTROLLER_STORAGE_ENABLED
#error Should be including Matter/MTRDeviceControllerParameters.h
#endif // MTR_PER_CONTROLLER_STORAGE_ENABLED

#define MTR_INTERNAL_INCLUDE
#import <Matter/MTRDeviceControllerParameters.h>
#undef MTR_INTERNAL_INCLUDE
