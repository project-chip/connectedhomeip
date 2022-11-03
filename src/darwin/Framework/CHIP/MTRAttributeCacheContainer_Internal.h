/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

#import "MTRAttributeCacheContainer.h"
#import "MTRDeviceControllerOverXPC.h"

#include <app/ClusterStateCache.h>

NS_ASSUME_NONNULL_BEGIN

@interface MTRAttributeCacheContainer ()

@property (atomic, readwrite, nullable) chip::app::ClusterStateCache * cppAttributeCache;
@property (nonatomic, readwrite, copy) NSNumber * deviceID;
@property (nonatomic, readwrite, weak, nullable) MTRDeviceControllerXPCConnection * xpcConnection;
@property (nonatomic, readwrite, strong, nullable) id<NSCopying> xpcControllerID;
@property (atomic, readwrite) BOOL shouldUseXPC;

@end

NS_ASSUME_NONNULL_END
