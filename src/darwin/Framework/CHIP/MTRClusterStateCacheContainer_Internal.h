/**
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

#import "MTRClusterStateCacheContainer.h"
#import "MTRDeviceControllerOverXPC.h"

#include <app/ClusterStateCache.h>

NS_ASSUME_NONNULL_BEGIN

@interface MTRClusterStateCacheContainer ()

@property (atomic, readwrite, nullable) chip::app::ClusterStateCache * cppClusterStateCache;
@property (nonatomic, readwrite, nullable) MTRBaseDevice * baseDevice;

@end

@interface MTRAttributeCacheContainer ()
@property (nonatomic, readonly) MTRClusterStateCacheContainer * realContainer;
@end

NS_ASSUME_NONNULL_END
