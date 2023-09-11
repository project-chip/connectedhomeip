/**
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <XCTest/XCTest.h>

#import "MTRTestPerControllerStorage.h"

#if MTR_PER_CONTROLLER_STORAGE_ENABLED

@interface MTRTestPerControllerStorage ()
@property (nonatomic, readonly) NSMutableDictionary<NSString *, NSData *> * storage;
@end

@implementation MTRTestPerControllerStorage

- (instancetype)initWithControllerID:(NSUUID *)controllerID
{
    if (!(self = [super init])) {
        return nil;
    }

    _storage = [[NSMutableDictionary alloc] init];
    _controllerID = controllerID;
    return self;
}

- (nullable id<NSSecureCoding>)controller:(MTRDeviceController *)controller
                              valueForKey:(NSString *)key
                            securityLevel:(MTRStorageSecurityLevel)securityLevel
                              sharingType:(MTRStorageSharingType)sharingType
{
    XCTAssertEqualObjects(_controllerID, controller.uniqueIdentifier);

    __auto_type * data = self.storage[key];
    if (data == nil) {
        return data;
    }

    NSError * error;
    id value = [NSKeyedUnarchiver unarchivedObjectOfClasses:MTRDeviceControllerStorageClasses() fromData:data error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(data);

    return value;
}

- (BOOL)controller:(MTRDeviceController *)controller
        storeValue:(id<NSSecureCoding>)value
            forKey:(NSString *)key
     securityLevel:(MTRStorageSecurityLevel)securityLevel
       sharingType:(MTRStorageSharingType)sharingType
{
    XCTAssertEqualObjects(_controllerID, controller.uniqueIdentifier);

    NSError * error;
    NSData * data = [NSKeyedArchiver archivedDataWithRootObject:value requiringSecureCoding:YES error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(data);

    self.storage[key] = data;
    return YES;
}

- (BOOL)controller:(MTRDeviceController *)controller
    removeValueForKey:(NSString *)key
        securityLevel:(MTRStorageSecurityLevel)securityLevel
          sharingType:(MTRStorageSharingType)sharingType
{
    XCTAssertEqualObjects(_controllerID, controller.uniqueIdentifier);
    self.storage[key] = nil;
    return YES;
}

@end

#endif // MTR_PER_CONTROLLER_STORAGE_ENABLED
