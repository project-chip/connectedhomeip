/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DoorLock {

static constexpr size_t kAliroReaderVerificationKeySize = 65;

static constexpr size_t kAliroReaderGroupIdentifierSize = 16;

static constexpr size_t kAliroReaderGroupSubIdentifierSize = 16;

static constexpr size_t kAliroGroupResolvingKeySize = 16;

static constexpr size_t kAliroProtocolVersionSize = 2;

static constexpr size_t kAliroSigningKeySize = 32;

/** @brief
 *  Defines methods for implementing application-specific logic for the door lock cluster.
 *  It defines the interfaces that a door lock should implement to support Aliro provisioning attributes.
 */

class Delegate
{
public:
    Delegate() = default;

    virtual ~Delegate() = default;

    /**
     * @brief Get the Aliro verification key component of the Reader's key pair.
     *
     * @param[out] verificationKey The MutableByteSpan to copy the verification key into. On success,
     *             the callee must update the length to the length of the copied data. If the value of
     *             the attribute is null, the callee must set the MutableByteSpan to empty.
     */
    virtual CHIP_ERROR GetAliroReaderVerificationKey(MutableByteSpan & verificationKey) = 0;

    /**
     * @brief Get the Aliro Reader's group identifier
     *
     * @param[out] groupIdentifier The MutableByteSpan to copy the group identifier into. On success,
     *             the callee must update the length to the length of the copied data. If the value of
     *             the attribute is null, the callee must set the MutableByteSpan to empty.
     */
    virtual CHIP_ERROR GetAliroReaderGroupIdentifier(MutableByteSpan & groupIdentifier) = 0;

    /**
     * @brief Get the Aliro Reader's group subidentifier
     *
     * @param[out] groupSubIdentifier  The MutableByteSpan to copy the group subidentifier into. On success,
     *             the callee must update the length to the length of the copied data. The MutableByteSpan
     *             must not be empty since the attribute is not nullable.
     */
    virtual CHIP_ERROR GetAliroReaderGroupSubIdentifier(MutableByteSpan & groupSubIdentifier) = 0;

    /**
     * @brief Get the Aliro expedited transaction supported protocol version at the given index.
     *
     * @param[in] index The index of the protocol version in the list.
     * @param[out] protocolVersion The MutableByteSpan to copy the expedited transaction supported protocol version at the given
     *             index into. On success, the callee must update the length to the length of the copied data.
     * @return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the index is out of range for the attribute list.
     */
    virtual CHIP_ERROR GetAliroExpeditedTransactionSupportedProtocolVersionAtIndex(size_t index,
                                                                                   MutableByteSpan & protocolVersion) = 0;

    /**
     * @brief Get the Reader's group resolving key.
     *
     * @param[out] groupResolvingKey The MutableByteSpan to copy the group resolving key into. On success,
     *             the callee must update the length to the length of the copied data. If the value of
     *             the attribute is null, the callee must set the MutableByteSpan to empty.
     */
    virtual CHIP_ERROR GetAliroGroupResolvingKey(MutableByteSpan & groupResolvingKey) = 0;

    /**
     * @brief Get the Aliro supported BLE UWB protocol version at the given index.
     *
     * @param[in] index The index of the protocol version in the list.
     * @param[out] protocolVersion  The MutableByteSpan to copy the supported BLE UWB protocol version at the given index into.
     *             On success, the callee must update the length to the length of the copied data.
     * @return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the index is out of range for the attribute list.
     */
    virtual CHIP_ERROR GetAliroSupportedBLEUWBProtocolVersionAtIndex(size_t index, MutableByteSpan & protocolVersion) = 0;

    /**
     * @brief Get the Aliro BLE Advertising Version.
     *
     * @return The BLE Advertising Version.
     */
    virtual uint8_t GetAliroBLEAdvertisingVersion() = 0;

    /**
     * @brief Get the maximum number of Aliro credential issuer keys supported.
     *
     * @return The max number of Aliro credential issuer keys supported.
     */
    virtual uint16_t GetNumberOfAliroCredentialIssuerKeysSupported() = 0;

    /**
     * @brief Get the maximum number of Aliro endpoint keys supported.
     *
     * @return The max number of Aliro endpoint keys supported.
     */
    virtual uint16_t GetNumberOfAliroEndpointKeysSupported() = 0;

    /**
     * Set the Aliro reader configuration for the lock.  The various arguments
     * have already been checked for constraints and consistency with the
     * FeatureMap.
     *
     * @param[in] signingKey Signing key component of the Reader's key pair.
     * @param[in] verificationKey Verification key component of the Reader's key pair.
     * @param[in] groupIdentifier Reader group identifier for the lock.
     * @param[in] groupResolvingKey Group resolving key for the lock if Aliro BLE UWB feature is supported
     */
    virtual CHIP_ERROR SetAliroReaderConfig(const ByteSpan & signingKey, const ByteSpan & verificationKey,
                                            const ByteSpan & groupIdentifier, const Optional<ByteSpan> & groupResolvingKey) = 0;

    /**
     * Clear the Aliro reader configuration for the lock.
     */
    virtual CHIP_ERROR ClearAliroReaderConfig() = 0;
};

} // namespace DoorLock
} // namespace Clusters
} // namespace app
} // namespace chip
