/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          Storage interface, this is a generic interface for storage
 *
 */


#ifndef CHIP_STORAGE_H
#define CHIP_STORAGE_H

#include <platform/PersistedStorage.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

using namespace ::chip::Platform::PersistedStorage;

/*
 * Read-only storage interface
 */
class ChipStorage {
public:
    virtual const char *GetName() = 0;
    virtual bool IsExists(Key key) = 0;
    virtual CHIP_ERROR ReadValue(Key key, bool & val) = 0;
    virtual CHIP_ERROR ReadValue(Key key, uint32_t & val) = 0;
    virtual CHIP_ERROR ReadValue(Key key, uint64_t & val) = 0;
    virtual CHIP_ERROR ReadValueStr(Key key, char * buf, size_t bufSize, size_t & outLen) = 0;
    virtual CHIP_ERROR ReadValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen) = 0;

protected:
    virtual ~ChipStorage() = 0;
};

inline ChipStorage::~ChipStorage() { }

/*
 * Mutable storage interface
 */
class ChipMutableStorage : public ChipStorage {
public:
    virtual CHIP_ERROR WriteValue(Key key, bool val) = 0;
    virtual CHIP_ERROR WriteValue(Key key, uint32_t val) = 0;
    virtual CHIP_ERROR WriteValue(Key key, uint64_t val) = 0;
    virtual CHIP_ERROR WriteValueStr(Key key, const char * str) = 0;
    virtual CHIP_ERROR WriteValueBin(Key key, const uint8_t * data, size_t dataLen) = 0;
    virtual CHIP_ERROR ClearValue(Key key) = 0;
    virtual CHIP_ERROR ClearAll() = 0;
    virtual CHIP_ERROR Commit() = 0;

protected:
    ~ChipMutableStorage() override = 0;
};

inline ChipMutableStorage::~ChipMutableStorage() { }

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_STORAGE_H
