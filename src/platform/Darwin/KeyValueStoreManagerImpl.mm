/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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

/**
 *    @file
 *          Platform-specific key value storage implementation for Darwin
 */

#if !__has_feature(objc_arc)
#error This file must be compiled with ARC. Use -fobjc-arc flag (or convert project to ARC).
#endif

#include <platform/KeyValueStoreManager.h>

#include <algorithm>

#include <lib/support/CodeUtils.h>

#import <CoreData/CoreData.h>
#import <CoreFoundation/CoreFoundation.h>

#ifndef CHIP_CONFIG_DARWIN_STORAGE_VERBOSE_LOGGING
#define CHIP_CONFIG_DARWIN_STORAGE_VERBOSE_LOGGING 0
#endif // CHIP_CONFIG_DARWIN_STORAGE_VERBOSE_LOGGING

@interface KeyValueItem : NSManagedObject

@property (nonatomic, retain) NSString * key;
@property (nonatomic, retain) NSData * value;

@end

@implementation KeyValueItem

@dynamic key;
@dynamic value;

- (instancetype)initWithContext:(nonnull NSManagedObjectContext *)context key:(nonnull NSString *)key value:(nonnull NSData *)value
{
    if (self = [super initWithContext:context]) {
        self.key = key;
        self.value = value;
    }
    return self;
}

@end

namespace chip {
namespace DeviceLayer {
    namespace PersistedStorage {
        namespace {

            NSManagedObjectContext * gContext = nullptr;

            NSManagedObjectModel * CreateManagedObjectModel()
            {
                NSManagedObjectModel * model = [[NSManagedObjectModel alloc] init];

                // create the entity
                NSEntityDescription * entity = [[NSEntityDescription alloc] init];
                [entity setName:@"KeyValue"];
                [entity setManagedObjectClassName:@"KeyValueItem"];

                // create the attributes
                NSMutableArray * properties = [NSMutableArray array];

                NSAttributeDescription * keyAttribute = [[NSAttributeDescription alloc] init];
                [keyAttribute setName:@"key"];
                [keyAttribute setAttributeType:NSStringAttributeType];
                [keyAttribute setOptional:NO];
                [properties addObject:keyAttribute];

                NSAttributeDescription * valueAttribute = [[NSAttributeDescription alloc] init];
                [valueAttribute setName:@"value"];
                [valueAttribute setAttributeType:NSBinaryDataAttributeType];
                [valueAttribute setOptional:NO];
                [properties addObject:valueAttribute];

                NSFetchIndexElementDescription * elementIndex =
                    [[NSFetchIndexElementDescription alloc] initWithProperty:keyAttribute
                                                               collationType:NSFetchIndexElementTypeBinary];
                elementIndex.ascending = true;

                NSFetchIndexDescription * keyIndexDescription =
                    [[NSFetchIndexDescription alloc] initWithName:@"kv_item_key"
                                                         elements:[[NSArray alloc] initWithObjects:elementIndex, nil]];

                // add attributes to entity
                [entity setProperties:properties];
                [entity setIndexes:[[NSArray alloc] initWithObjects:keyIndexDescription, nil]];

                // add entity to model
                [model setEntities:[NSArray arrayWithObject:entity]];

                return model;
            }

            KeyValueItem * FindItemForKey(NSString * key, NSError ** error, BOOL returnsData)
            {
                NSFetchRequest * request = [[NSFetchRequest alloc] initWithEntityName:@"KeyValue"];
                if (returnsData) {
                    [request setReturnsObjectsAsFaults:NO];
                }
                request.predicate = [NSPredicate predicateWithFormat:@"key = %@", key];

                __block NSError * fetchError = nil;
                __block NSArray * result;
                [gContext performBlockAndWait:^{
                    result = [gContext executeFetchRequest:request error:&fetchError];
                }];

                if (error != nil) {
                    *error = fetchError;
                }

                if (result == nil) {
                    return nullptr;
                }

                if (result.count == 0) {
                    return nullptr;
                }
                return (KeyValueItem *) [result objectAtIndex:0];
            }

            KeyValueItem * FindItemForKey(NSString * key, NSError ** error) { return FindItemForKey(key, error, false); }

        } // namespace

        KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

        CHIP_ERROR KeyValueStoreManagerImpl::Init(const char * fileName)
        {
            @autoreleasepool {
                if (mInitialized) {
                    return CHIP_NO_ERROR;
                }

                ReturnErrorCodeIf(gContext != nullptr, CHIP_ERROR_INCORRECT_STATE);
                ReturnErrorCodeIf(fileName == nullptr, CHIP_ERROR_INVALID_ARGUMENT);
                ReturnErrorCodeIf(fileName[0] == '\0', CHIP_ERROR_INVALID_ARGUMENT);

                NSURL * url = nullptr;
                NSString * filepath = [NSString stringWithUTF8String:fileName];
                ReturnErrorCodeIf(filepath == nil, CHIP_ERROR_INVALID_ARGUMENT);

                // relative paths are relative to Documents folder
                if (![filepath hasPrefix:@"/"]) {
                    NSURL * documentsDirectory = [NSFileManager.defaultManager URLForDirectory:NSDocumentDirectory
                                                                                      inDomain:NSUserDomainMask
                                                                             appropriateForURL:nil
                                                                                        create:YES
                                                                                         error:nil];
                    if (documentsDirectory == nullptr) {
                        ChipLogError(DeviceLayer, "Failed to get documents directory.");
                        return CHIP_ERROR_INTERNAL;
                    }
                    ChipLogProgress(
                        DeviceLayer, "Found user documents directory: %s", [[documentsDirectory absoluteString] UTF8String]);

                    url = [NSURL URLWithString:filepath relativeToURL:documentsDirectory];
                } else {
                    url = [NSURL fileURLWithPath:filepath];
                }
                ReturnErrorCodeIf(url == nullptr, CHIP_ERROR_NO_MEMORY);

                ChipLogProgress(DeviceLayer, "KVS will be written to: %s", [[url absoluteString] UTF8String]);

                NSManagedObjectModel * model = CreateManagedObjectModel();
                ReturnErrorCodeIf(model == nullptr, CHIP_ERROR_NO_MEMORY);

                // setup persistent store coordinator

                NSPersistentStoreCoordinator * coordinator = [[NSPersistentStoreCoordinator alloc] initWithManagedObjectModel:model];

                NSError * error = nil;
                if (![coordinator addPersistentStoreWithType:NSSQLiteStoreType configuration:nil URL:url options:nil error:&error]) {
                    ChipLogError(DeviceLayer, "Invalid store. Attempting to clear: %s", error.localizedDescription.UTF8String);
                    if (![[NSFileManager defaultManager] removeItemAtURL:url error:&error]) {
                        ChipLogError(DeviceLayer, "Failed to delete item: %s", error.localizedDescription.UTF8String);
                    }

                    if (![coordinator addPersistentStoreWithType:NSSQLiteStoreType
                                                   configuration:nil
                                                             URL:url
                                                         options:nil
                                                           error:&error]) {
                        ChipLogError(DeviceLayer, "Failed to initialize clear KVS storage: %s", error.localizedDescription.UTF8String);
                        chipDie();
                    }
                }

                // create Managed Object context
                gContext = [[NSManagedObjectContext alloc] initWithConcurrencyType:NSPrivateQueueConcurrencyType];
                [gContext setMergePolicy:NSMergeByPropertyObjectTrumpMergePolicy];
                [gContext setPersistentStoreCoordinator:coordinator];

                mInitialized = true;
                return CHIP_NO_ERROR;
            } // @autoreleasepool
        }

        CHIP_ERROR KeyValueStoreManagerImpl::_Get(
            const char * key, void * value, size_t value_size, size_t * read_bytes_size, size_t offset)
        {
            @autoreleasepool {
                ReturnErrorCodeIf(key == nullptr, CHIP_ERROR_INVALID_ARGUMENT);
                ReturnErrorCodeIf(offset != 0, CHIP_ERROR_INVALID_ARGUMENT);
                ReturnErrorCodeIf(gContext == nullptr, CHIP_ERROR_UNINITIALIZED);

                KeyValueItem * item = FindItemForKey([[NSString alloc] initWithUTF8String:key], nil, true);
                if (!item) {
                    return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
                }

                __block NSData * itemValue = nil;
                // can only access this object on the managed queue
                [gContext performBlockAndWait:^{
                    itemValue = item.value;
                }];

                if (read_bytes_size != nullptr) {
                    *read_bytes_size = itemValue.length;
                }

                if (value != nullptr) {
                    memcpy(value, itemValue.bytes, std::min<size_t>((itemValue.length), value_size));
#if CHIP_CONFIG_DARWIN_STORAGE_VERBOSE_LOGGING
                    fprintf(stderr, "GETTING VALUE FOR: '%s': ", key);
                    for (size_t i = 0; i < std::min<size_t>((itemValue.length), value_size); ++i) {
                        fprintf(stderr, "%02x ", static_cast<uint8_t *>(value)[i]);
                    }
                    fprintf(stderr, "\n");
#endif
                }

                if (itemValue.length > value_size) {
                    return CHIP_ERROR_BUFFER_TOO_SMALL;
                }

                return CHIP_NO_ERROR;
            } // @autoreleasepool
        }

        CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
        {
            @autoreleasepool {
                ReturnErrorCodeIf(key == nullptr, CHIP_ERROR_INVALID_ARGUMENT);
                ReturnErrorCodeIf(gContext == nullptr, CHIP_ERROR_UNINITIALIZED);

                KeyValueItem * item = FindItemForKey([[NSString alloc] initWithUTF8String:key], nil);
                if (!item) {
                    return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
                }

                __block BOOL success = NO;
                __block NSError * error = nil;
                [gContext performBlockAndWait:^{
                    [gContext deleteObject:item];
                    success = [gContext save:&error];
                }];

                if (!success) {
                    ChipLogError(DeviceLayer, "Error saving context: %s", error.localizedDescription.UTF8String);
                    return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
                }

                return CHIP_NO_ERROR;
            } // @autoreleasepool
        }

        CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
        {
            @autoreleasepool {
                ReturnErrorCodeIf(key == nullptr, CHIP_ERROR_INVALID_ARGUMENT);
                ReturnErrorCodeIf(gContext == nullptr, CHIP_ERROR_UNINITIALIZED);

                NSData * data = [[NSData alloc] initWithBytes:value length:value_size];

                NSString * itemKey = [[NSString alloc] initWithUTF8String:key];
                ReturnErrorCodeIf(itemKey == nil, CHIP_ERROR_INVALID_ARGUMENT);

                KeyValueItem * item = FindItemForKey(itemKey, nil);
                if (!item) {
                    [gContext performBlockAndWait:^{
                        [gContext insertObject:[[KeyValueItem alloc] initWithContext:gContext key:itemKey value:data]];
                    }];
                } else {
                    [gContext performBlockAndWait:^{
                        item.value = data;
                    }];
                }

                __block BOOL success = NO;
                __block NSError * error = nil;
                [gContext performBlockAndWait:^{
                    success = [gContext save:&error];
                }];

                if (!success) {
                    ChipLogError(DeviceLayer, "Error saving context: %s", error.localizedDescription.UTF8String);
                    return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
                }

#if CHIP_CONFIG_DARWIN_STORAGE_VERBOSE_LOGGING
                fprintf(stderr, "PUT VALUE FOR: '%s': ", key);
                for (size_t i = 0; i < value_size; ++i) {
                    fprintf(stderr, "%02x ", static_cast<const uint8_t *>(value)[i]);
                }
                fprintf(stderr, "\n");
#endif

                return CHIP_NO_ERROR;
            } // @autoreleasepool
        }

    } // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
