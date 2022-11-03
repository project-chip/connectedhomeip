/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRMemory.h"

#include <lib/support/CHIPMem.h>

@implementation MTRMemory

+ (void)ensureInit
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        // The malloc version of MemoryInit never fails.
        chip::Platform::MemoryInit();
    });
}

@end
