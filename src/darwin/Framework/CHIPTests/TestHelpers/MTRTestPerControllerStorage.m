/**
 *    Copyright (c) 2023 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
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
