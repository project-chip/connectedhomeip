/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>
#import <Matter/Matter.h>

NS_ASSUME_NONNULL_BEGIN

@interface MTRTestKeys : NSObject <MTRKeypair>

@property (readonly, nonatomic, strong) NSData * ipk;

- (instancetype)init;

@end

NS_ASSUME_NONNULL_END
