/*
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

#include <app/server/Server.h>
#include <platform/KeyValueStoreManager.h>

namespace chip {

class DefaultStorageDelegate : public PersistentStorageDelegate
{
public:
    CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) override
    {
        size_t bytesRead = 0;
        CHIP_ERROR err   = DeviceLayer::PersistedStorage::KeyValueStoreMgr().Get(key, buffer, size, &bytesRead);

        if (err == CHIP_NO_ERROR)
        {
            ChipLogProgress(AppServer, "Retrieved from server storage: %s", key);
        }
        size = static_cast<uint16_t>(bytesRead);
        return err;
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

PersistentStorageDelegate & __attribute__((weak)) Server::GetPersistentStorageDelegate()
{
    static DefaultStorageDelegate storage;
    return storage;
}

} // namespace chip
