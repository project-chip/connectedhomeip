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

@interface KeyValueItem : NSManagedObject

@property (nonatomic, retain) NSString * key;
@property (nonatomic, retain) NSData * value;

@end

@implementation KeyValueItem

@synthesize key;
@synthesize value;

- (instancetype)initWithContext:(nonnull NSManagedObjectContext *)context
                            key:(nonnull NSString *)key_
                          value:(nonnull NSData *)value_
{
    if (self = [super initWithContext:context]) {
        key = key_;
        value = value_;
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

            KeyValueItem * FindItemForKey(NSString * key, NSError ** error)
            {
                NSFetchRequest * request = [[NSFetchRequest alloc] initWithEntityName:@"KeyValue"];
                request.predicate = [NSPredicate predicateWithFormat:@"key = %@", key];

                NSArray * result = [gContext executeFetchRequest:request error:error];
                if (result == nil) {
                    return nullptr;
                }

                if (result.count == 0) {
                    return nullptr;
                }
                return (KeyValueItem *) [result objectAtIndex:0];
            }

        }

        KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

        CHIP_ERROR KeyValueStoreManagerImpl::Init(const char * fileName)
        {
            ReturnErrorCodeIf(gContext != nullptr, CHIP_ERROR_INCORRECT_STATE);
            ReturnErrorCodeIf(fileName == nullptr, CHIP_ERROR_INVALID_ARGUMENT);
            ReturnErrorCodeIf(fileName[0] == '\0', CHIP_ERROR_INVALID_ARGUMENT);

            NSURL * url = nullptr;

            // relative paths are relative to Documents folder
            if (fileName[0] != '/') {
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

                url = [NSURL URLWithString:[NSString stringWithUTF8String:fileName] relativeToURL:documentsDirectory];
            } else {
                url = [NSURL URLWithString:[NSString stringWithUTF8String:fileName]];
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
            gContext = [[NSManagedObjectContext alloc] initWithConcurrencyType:NSMainQueueConcurrencyType];
            [gContext setPersistentStoreCoordinator:coordinator];

            return CHIP_NO_ERROR;
        }

        CHIP_ERROR KeyValueStoreManagerImpl::_Get(
            const char * key, void * value, size_t value_size, size_t * read_bytes_size, size_t offset)
        {
            ReturnErrorCodeIf(key == nullptr, CHIP_ERROR_INVALID_ARGUMENT);
            ReturnErrorCodeIf(offset != 0, CHIP_ERROR_INVALID_ARGUMENT);

            KeyValueItem * item = FindItemForKey([[NSString alloc] initWithUTF8String:key], nil);
            if (!item) {
                return CHIP_ERROR_KEY_NOT_FOUND;
            }

            if (read_bytes_size != nullptr) {
                *read_bytes_size = item.value.length;
            }

            if (value != nullptr) {
                memcpy(value, item.value.bytes, std::min<size_t>((item.value.length), value_size));
            }

            return CHIP_NO_ERROR;
        }

        CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
        {
            ReturnErrorCodeIf(key == nullptr, CHIP_ERROR_INVALID_ARGUMENT);

            KeyValueItem * item = FindItemForKey([[NSString alloc] initWithUTF8String:key], nil);
            if (!item) {
                return CHIP_NO_ERROR;
            }

            [gContext deleteObject:item];

            NSError * error = nil;
            if (![gContext save:&error]) {
                ChipLogError(DeviceLayer, "Error saving context: %s", error.localizedDescription.UTF8String);
                return CHIP_ERROR_INTERNAL;
            }

            return CHIP_NO_ERROR;
        }

        CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
        {
            ReturnErrorCodeIf(key == nullptr, CHIP_ERROR_INVALID_ARGUMENT);

            NSData * data = [[NSData alloc] initWithBytes:value length:value_size];

            KeyValueItem * item = FindItemForKey([[NSString alloc] initWithUTF8String:key], nil);
            if (!item) {
                item = [[KeyValueItem alloc] initWithContext:gContext key:[[NSString alloc] initWithUTF8String:key] value:data];
                [gContext insertObject:item];
            } else {
                item.value = data;
            }

            NSError * error = nil;
            if (![gContext save:&error]) {
                ChipLogError(DeviceLayer, "Error saving context: %s", error.localizedDescription.UTF8String);
                return CHIP_ERROR_INTERNAL;
            }

            return CHIP_NO_ERROR;
        }

    } // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
