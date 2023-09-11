/**
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>
#import <Matter/Matter.h>

#if MTR_PER_CONTROLLER_STORAGE_ENABLED

NS_ASSUME_NONNULL_BEGIN

@interface MTRTestPerControllerStorage : NSObject <MTRDeviceControllerStorageDelegate>

- (instancetype)initWithControllerID:(NSUUID *)controllerID;

@property (nonatomic, readonly) NSUUID * controllerID;

- (nullable id<NSSecureCoding>)controller:(MTRDeviceController *)controller
                              valueForKey:(NSString *)key
                            securityLevel:(MTRStorageSecurityLevel)securityLevel
                              sharingType:(MTRStorageSharingType)sharingType;
- (BOOL)controller:(MTRDeviceController *)controller
        storeValue:(id<NSSecureCoding>)value
            forKey:(NSString *)key
     securityLevel:(MTRStorageSecurityLevel)securityLevel
       sharingType:(MTRStorageSharingType)sharingType;
- (BOOL)controller:(MTRDeviceController *)controller
    removeValueForKey:(NSString *)key
        securityLevel:(MTRStorageSecurityLevel)securityLevel
          sharingType:(MTRStorageSharingType)sharingType;
@end

NS_ASSUME_NONNULL_END

#endif // MTR_PER_CONTROLLER_STORAGE_ENABLED
