/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#pragma once

#include <lib/support/CHIPMemString.h>
#include <stdio.h>

namespace chip {

/**
 * Represents a key used for addressing a specific storage element.
 *
 * May contain generic fixed keys (e.g. "g/fidx") or formatted fabric-specific
 * keys ("f/%x/..." where %x is the fabric index).
 */
class StorageKeyName
{
public:
    StorageKeyName(const StorageKeyName & other)             = default;
    StorageKeyName & operator=(const StorageKeyName & other) = default;

    ~StorageKeyName() { memset(mKeyNameBuffer, 0, sizeof(mKeyNameBuffer)); }

    const char * KeyName() const { return mKeyNameBuffer; }

    bool IsInitialized() const { return mKeyNameBuffer[0] != 0; }
    bool IsUninitialized() const { return mKeyNameBuffer[0] == 0; }
    bool operator!() const { return IsUninitialized(); }

    static StorageKeyName FromConst(const char * value)
    {
        StorageKeyName result;
        Platform::CopyString(result.mKeyNameBuffer, value);
        return result;
    }

    static StorageKeyName ENFORCE_FORMAT(1, 2) Formatted(const char * format, ...)
    {
        StorageKeyName result;

        va_list args;
        va_start(args, format);
        vsnprintf(result.mKeyNameBuffer, sizeof(result.mKeyNameBuffer), format, args);
        va_end(args);

        return result;
    }

    // Explicit 0-filled key. MUST be initialized later
    static StorageKeyName Uninitialized()
    {
        StorageKeyName result;
        return result;
    }

private:
    // May only be created by the underlying constructor methods
    StorageKeyName() {}

    // Contains the storage for the key name because some strings may be formatted.
    char mKeyNameBuffer[PersistentStorageDelegate::kKeyLengthMax + 1] = { 0 };
};

} // namespace chip
