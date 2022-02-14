/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

/* This file contains the Storage interface for the OTARequestor
 */

#pragma once

namespace chip {
class OTARequestorStorage : public PersistentStorageDelegate
{
public:
    virtual ~OTARequestorStorage() = default;

    // Reads a KVS value from an offset if desired and returns number of bytes read
    // TODO: Move to PersistentStorageDelegate interface ideally
    virtual CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size, size_t * read_bytes_size,
                                       size_t offset_bytes) = 0;
};
} // namespace chip
