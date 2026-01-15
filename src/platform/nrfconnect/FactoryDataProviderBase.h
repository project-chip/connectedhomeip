/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <credentials/DeviceAttestationCredsProvider.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/DeviceInstanceInfoProvider.h>

namespace chip {
namespace DeviceLayer {

/**
 * @brief Base class for factory data providers.
 *
 * This class provides a base interface for factory data providers.
 * It implements the DeviceAttestationCredentialsProvider, CommissionableDataProvider, and DeviceInstanceInfoProvider interfaces
 * and can be used to
 */
class FactoryDataProviderBase : public chip::Credentials::DeviceAttestationCredentialsProvider,
                                public CommissionableDataProvider,
                                public DeviceInstanceInfoProvider
{
public:
    /**
     * @brief Perform all operations needed to initialize factory data provider.
     *
     * @returns CHIP_NO_ERROR in case of a success, specific error code otherwise
     */
    virtual CHIP_ERROR Init() = 0;

    /**
     * @brief Get the EnableKey as MutableByteSpan
     *
     * @param enableKey MutableByteSpan object to obtain EnableKey
     * @returns
     * CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND if factory data does not contain enable_key field, or the value cannot be read
     * out. CHIP_ERROR_BUFFER_TOO_SMALL if provided MutableByteSpan is too small
     * purposes.
     */
    virtual CHIP_ERROR GetEnableKey(MutableByteSpan & enableKey) = 0;

    /**
     * @brief Get the user data as MutableByteSpan
     *
     * @param userData MutableByteSpan object to obtain all user data
     * @returns
     * CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND if factory data does not contain user field, or the value cannot be read out.
     * CHIP_ERROR_BUFFER_TOO_SMALL if provided MutableByteSpan is too small
     * CHIP_ERROR_NOT_IMPLEMENTED if the platform does not support user data
     */
    virtual CHIP_ERROR GetUserData(MutableByteSpan & userData) { return CHIP_ERROR_NOT_IMPLEMENTED; };

    /**
     * @brief Try to find user data key and return its value
     *
     * @param userKey A key name to be found
     * @param buf Buffer to store value of found key
     * @param len Length of the buffer. This value will be updated to the actual value if the key is read.
     * @returns
     * CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND if factory data does not contain user key field, or the value cannot be read
     * out. CHIP_ERROR_BUFFER_TOO_SMALL if provided buffer length is too small
     * CHIP_ERROR_NOT_IMPLEMENTED if the platform does not support user data
     */
    virtual CHIP_ERROR GetUserKey(const char * userKey, void * buf, size_t & len) { return CHIP_ERROR_NOT_IMPLEMENTED; };
};

} // namespace DeviceLayer
} // namespace chip
