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
#include <credentials/FabricTable.h>

namespace chip {

class DefaultFabricStorage : public FabricStorage
{
public:
    DefaultFabricStorage(PersistentStorageDelegate & storage) : mStorage(storage) {}

    CHIP_ERROR SyncStore(FabricIndex fabricIndex, const char * key, const void * buffer, uint16_t size) override
    {
        return mStorage.SyncSetKeyValue(key, buffer, size);
    };

    CHIP_ERROR SyncLoad(FabricIndex fabricIndex, const char * key, void * buffer, uint16_t & size) override
    {
        return mStorage.SyncGetKeyValue(key, buffer, size);
    };

    CHIP_ERROR SyncDelete(FabricIndex fabricIndex, const char * key) override { return mStorage.SyncDeleteKeyValue(key); };

private:
    PersistentStorageDelegate & mStorage;
};

FabricStorage & __attribute__((weak)) Server::GetFabricStorage()
{
    static DefaultFabricStorage fabricStorage(GetPersistentStorageDelegate());
    return fabricStorage;
}

} // namespace chip
