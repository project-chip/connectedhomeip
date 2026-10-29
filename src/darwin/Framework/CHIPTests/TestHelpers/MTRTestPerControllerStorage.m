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

@interface MTRTestPerControllerStorage ()
@property (nonatomic, readonly) NSMutableDictionary<NSString *, NSData *> * storage;
@property (nonatomic, readonly) NSMutableArray<NSString *> * mutableSingleStoreKeys;
@property (nonatomic, readonly) NSMutableArray<NSSet<NSString *> *> * mutableBulkStoreKeys;
@property (nonatomic, readonly) NSMutableArray<NSString *> * mutableRemovedKeys;
@property (nonatomic) NSInteger writesBeforeTermination;
- (BOOL)takeWrite;
- (void)archiveValue:(id<NSSecureCoding>)value forKey:(NSString *)key;
@end

@implementation MTRTestPerControllerStorage

- (instancetype)initWithControllerID:(NSUUID *)controllerID
{
    if (!(self = [super init])) {
        return nil;
    }

    _storage = [[NSMutableDictionary alloc] init];
    _mutableSingleStoreKeys = [[NSMutableArray alloc] init];
    _mutableBulkStoreKeys = [[NSMutableArray alloc] init];
    _mutableRemovedKeys = [[NSMutableArray alloc] init];
    _controllerID = controllerID;
    _writesBeforeTermination = -1;
    return self;
}

- (nullable id<NSSecureCoding>)controller:(MTRDeviceController *)controller
                              valueForKey:(NSString *)key
                            securityLevel:(MTRStorageSecurityLevel)securityLevel
                              sharingType:(MTRStorageSharingType)sharingType
{
    @synchronized(self) {
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
}

- (BOOL)controller:(MTRDeviceController *)controller
        storeValue:(id<NSSecureCoding>)value
            forKey:(NSString *)key
     securityLevel:(MTRStorageSecurityLevel)securityLevel
       sharingType:(MTRStorageSharingType)sharingType
{
    @synchronized(self) {
        XCTAssertEqualObjects(_controllerID, controller.uniqueIdentifier);

        [self.mutableSingleStoreKeys addObject:key];
        if (![self takeWrite]) {
            return NO;
        }
        if (self.shouldFailStore && self.shouldFailStore([NSSet setWithObject:key])) {
            return NO;
        }
        [self archiveValue:value forKey:key];
        return YES;
    }
}

- (void)archiveValue:(id<NSSecureCoding>)value forKey:(NSString *)key
{
    NSError * error;
    NSData * data = [NSKeyedArchiver archivedDataWithRootObject:value requiringSecureCoding:YES error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(data);

    self.storage[key] = data;
}

- (BOOL)controller:(MTRDeviceController *)controller
    removeValueForKey:(NSString *)key
        securityLevel:(MTRStorageSecurityLevel)securityLevel
          sharingType:(MTRStorageSharingType)sharingType
{
    @synchronized(self) {
        XCTAssertEqualObjects(_controllerID, controller.uniqueIdentifier);
        [self.mutableRemovedKeys addObject:key];
        if (![self takeWrite]) {
            return NO;
        }
        self.storage[key] = nil;
        return YES;
    }
}

- (NSUInteger)count
{
    @synchronized(self) {
        return self.storage.count;
    }
}

- (NSArray<NSString *> *)singleStoreKeys
{
    @synchronized(self) {
        return [self.mutableSingleStoreKeys copy];
    }
}

- (NSArray<NSSet<NSString *> *> *)bulkStoreKeys
{
    @synchronized(self) {
        return [self.mutableBulkStoreKeys copy];
    }
}

- (NSArray<NSString *> *)removedKeys
{
    @synchronized(self) {
        return [self.mutableRemovedKeys copy];
    }
}

- (BOOL)takeWrite
{
    if (self.writesBeforeTermination == 0) {
        return NO;
    }
    if (self.writesBeforeTermination > 0) {
        self.writesBeforeTermination--;
    }
    return YES;
}

- (void)terminateAfterWrites:(NSUInteger)writeCount
{
    @synchronized(self) {
        self.writesBeforeTermination = (NSInteger) writeCount;
    }
}

- (void)resumeAfterTermination
{
    @synchronized(self) {
        self.writesBeforeTermination = -1;
    }
}

- (void)resetRecordedCalls
{
    @synchronized(self) {
        [self.mutableSingleStoreKeys removeAllObjects];
        [self.mutableBulkStoreKeys removeAllObjects];
        [self.mutableRemovedKeys removeAllObjects];
    }
}

@end

@implementation MTRTestPerControllerStorageWithBulkReadWrite

- (NSDictionary<NSString *, id<NSSecureCoding>> *)valuesForController:(MTRDeviceController *)controller securityLevel:(MTRStorageSecurityLevel)securityLevel sharingType:(MTRStorageSharingType)sharingType
{
    @synchronized(self) {
        XCTAssertEqualObjects(self.controllerID, controller.uniqueIdentifier);

        if (!self.storage.count) {
            return nil;
        }

        NSMutableDictionary * valuesToReturn = [NSMutableDictionary dictionary];
        for (NSString * key in self.storage) {
            valuesToReturn[key] = [self controller:controller valueForKey:key securityLevel:securityLevel sharingType:sharingType];
        }

        return valuesToReturn;
    }
}

- (BOOL)controller:(MTRDeviceController *)controller storeValues:(NSDictionary<NSString *, id<NSSecureCoding>> *)values securityLevel:(MTRStorageSecurityLevel)securityLevel sharingType:(MTRStorageSharingType)sharingType
{
    @synchronized(self) {
        XCTAssertEqualObjects(self.controllerID, controller.uniqueIdentifier);

        NSSet<NSString *> * keys = [NSSet setWithArray:values.allKeys];
        [self.mutableBulkStoreKeys addObject:keys];
        if (![self takeWrite]) {
            return NO;
        }
        if (self.shouldFailStore && self.shouldFailStore(keys)) {
            return NO;
        }
        for (NSString * key in values) {
            [self archiveValue:values[key] forKey:key];
        }

        return YES;
    }
}

@end
