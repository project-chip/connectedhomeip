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

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/nxp/k32w/common/RamStorage.h>
#include <platform/nxp/k32w/common/RamStorageKey.h>

namespace chip::DeviceLayer::Internal {

RamStorageKey::RamStorageKey(RamStorage * storage, uint8_t keyId, uint8_t pdmId)
{
    mStorage = storage;
    mId      = GetPdmId(keyId, pdmId);
}

CHIP_ERROR RamStorageKey::Read(uint8_t * buf, uint16_t & sizeToRead) const
{
    return mStorage->Read(mId, 0, buf, &sizeToRead);
}

CHIP_ERROR RamStorageKey::Write(const uint8_t * buf, uint16_t length)
{
    return mStorage->Write(mId, buf, length);
}

CHIP_ERROR RamStorageKey::Delete()
{
    return mStorage->Delete(mId, -1);
}

} // namespace chip::DeviceLayer::Internal
