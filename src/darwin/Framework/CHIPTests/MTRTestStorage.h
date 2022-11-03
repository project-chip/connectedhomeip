/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>
#import <Matter/Matter.h>

NS_ASSUME_NONNULL_BEGIN

@interface MTRTestStorage : NSObject <MTRPersistentStorageDelegate>
- (nullable NSData *)storageDataForKey:(NSString *)key;
- (BOOL)setStorageData:(NSData *)value forKey:(NSString *)key;
- (BOOL)removeStorageDataForKey:(NSString *)key;
@end

NS_ASSUME_NONNULL_END
