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

#import <Foundation/Foundation.h>
#import <Matter/MTRDeviceControllerFactory.h>

#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/DataModelTypes.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * A PersistentStorageDelegate implementation that does the following:
 *
 * 1) Ensures that any "global" storage keys are stored in RAM as needed so that
 *    the Matter stack has access to them.
 * 2) Hands off fabric-index-specific keys to the controller that corresponds to
 *    that fabric index, if any.
 */
class MTRDemuxingStorage : public chip::PersistentStorageDelegate {
public:
    MTRDemuxingStorage(MTRDeviceControllerFactory * factory);
    ~MTRDemuxingStorage() {}

    // PersistentStorageDelegate API.
    CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) override;

    CHIP_ERROR SyncSetKeyValue(const char * key, const void * value, uint16_t size) override;

    CHIP_ERROR SyncDeleteKeyValue(const char * key) override;

private:
    /**
     * Methods for reading/writing/deleting things.  The index-specific ones
     * will have the "f/index/" bit already stripped of from the front of the key.
     */
    NSData * _Nullable GetGlobalValue(NSString * key);
    NSData * _Nullable GetIndexSpecificValue(chip::FabricIndex index, NSString * key);

    CHIP_ERROR SetGlobalValue(NSString * key, NSData * data);
    CHIP_ERROR SetIndexSpecificValue(chip::FabricIndex index, NSString * key, NSData * data);

    CHIP_ERROR DeleteGlobalValue(NSString * key);
    CHIP_ERROR DeleteIndexSpecificValue(chip::FabricIndex index, NSString * key);

    /**
     * Methods for modifying our in-memory store for fully qualified keys.
     */
    NSData * _Nullable GetInMemoryValue(NSString * key);
    CHIP_ERROR SetInMemoryValue(NSString * key, NSData * data);
    CHIP_ERROR DeleteInMemoryValue(NSString * key);

    MTRDeviceControllerFactory * mFactory;
    NSMutableDictionary<NSString *, NSData *> * mInMemoryStore;
};

NS_ASSUME_NONNULL_END
