/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
/* This file contains a generic implementation for the OTARequestorStorage
 */
#include <app/clusters/ota-requestor/OTARequestorStorage.h>

namespace chip {
class GenericOTARequestorStorage : public OTARequestorStorage
{

    CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) override
    {
        return DeviceLayer::PersistedStorage::KeyValueStoreMgr().Get(key, buffer, size);
    }

    CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size, size_t * read_bytes_size, size_t offset_bytes)
    {
        return DeviceLayer::PersistedStorage::KeyValueStoreMgr().Get(key, buffer, size, read_bytes_size, offset_bytes);
    }

    CHIP_ERROR SyncSetKeyValue(const char * key, const void * value, uint16_t size) override
    {
        return DeviceLayer::PersistedStorage::KeyValueStoreMgr().Put(key, value, size);
    }

    CHIP_ERROR SyncDeleteKeyValue(const char * key) override
    {
        return DeviceLayer::PersistedStorage::KeyValueStoreMgr().Delete(key);
    }
};
} // namespace chip
