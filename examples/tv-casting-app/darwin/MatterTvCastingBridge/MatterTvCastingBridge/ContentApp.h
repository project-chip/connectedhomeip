/**
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

#ifndef ContentApp_h
#define ContentApp_h

@interface ContentApp : NSObject

@property uint16_t endpointId;

@property NSMutableArray * clusterIds;

/**
 @brief true, if all the fields are initialized, false otherwise
 */
@property BOOL isInitialized;

- (instancetype)initWithEndpointId:(uint16_t)endpointId clusterIds:(NSMutableArray *)clusterIds;

- (BOOL)supportsClusterWithId:(uint32_t)clusterId;

- (BOOL)supportsApplicationLauncher;

- (BOOL)supportsContentLauncher;

- (BOOL)supportsMediaPlayback;

- (BOOL)supportsLevelControl;

- (BOOL)supportsTargetNavigator;

@end

#endif /* ContentApp_h */
