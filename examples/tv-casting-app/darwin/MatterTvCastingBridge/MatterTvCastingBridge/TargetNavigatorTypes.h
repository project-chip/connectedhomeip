/**
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

#ifndef TargetNavigatorTypes_h
#define TargetNavigatorTypes_h

@interface TargetNavigator_TargetInfoStruct : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull identifier;
@property (nonatomic, copy) NSString * _Nonnull name;

- (TargetNavigator_TargetInfoStruct * _Nonnull)initWithIdentifier:(NSNumber * _Nonnull)identifier name:(NSString * _Nonnull)name;

- (id _Nonnull)copyWithZone:(NSZone * _Nullable)zone;
@end

#endif /* TargetNavigatorTypes_h */
