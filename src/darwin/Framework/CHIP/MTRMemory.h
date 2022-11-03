/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

/**
 * Utility to initialize the Matter memory subsystem.  Not a public framework
 * header.
 */

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface MTRMemory : NSObject
/**
 * Ensure Matter Platform::Memory is initialized.  This only needs to happen
 * once per process, because in practice we just use malloc/free so there is
 * nothing to initialize, so this just needs to happen to avoid debug
 * assertions.  This class handles ensuring the initialization only happens
 * once.
 */
+ (void)ensureInit;

@end

NS_ASSUME_NONNULL_END
