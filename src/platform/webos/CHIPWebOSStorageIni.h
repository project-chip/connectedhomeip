/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Provides an implementation of the Configuration key-value store interface
 *          using IniPP.
 *
 */

#pragma once

#include <inipp/inipp.h>
#include <lib/support/ScopedBuffer.h>
#include <platform/PersistedStorage.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

class ChipLinuxStorageIni
{
public:
    CHIP_ERROR Init();
    CHIP_ERROR AddConfig(const std::string & configFile);
    CHIP_ERROR CommitConfig(const std::string & configFile);
    CHIP_ERROR GetUInt16Value(const char * key, uint16_t & val);
    CHIP_ERROR GetUIntValue(const char * key, uint32_t & val);
    CHIP_ERROR GetUInt64Value(const char * key, uint64_t & val);
    CHIP_ERROR GetStringValue(const char * key, char * buf, size_t bufSize, size_t & outLen);
    CHIP_ERROR GetBinaryBlobValue(const char * key, uint8_t * decodedData, size_t bufSize, size_t & decodedDataLen);
    bool HasValue(const char * key);

protected:
    CHIP_ERROR AddEntry(const char * key, const char * value);
    CHIP_ERROR RemoveEntry(const char * key);
    CHIP_ERROR RemoveAll();

private:
    CHIP_ERROR GetBinaryBlobDataAndLengths(const char * key, chip::Platform::ScopedMemoryBuffer<char> & encodedData,
                                           size_t & encodedDataLen, size_t & decodedDataLen);
    inipp::Ini<char> mConfigStore;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
