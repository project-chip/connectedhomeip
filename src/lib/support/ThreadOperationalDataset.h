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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <lib/core/CHIPCore.h>
#include <lib/support/Span.h>

namespace chip {
namespace Thread {

class ThreadTLV;

inline constexpr size_t kChannel_NotSpecified = UINT8_MAX;
inline constexpr size_t kPANId_NotSpecified   = UINT16_MAX;

inline constexpr size_t kSizeOperationalDataset = 254;

inline constexpr size_t kSizeNetworkName     = 16;
inline constexpr size_t kSizeExtendedPanId   = 8;
inline constexpr size_t kSizeMasterKey       = 16;
inline constexpr size_t kSizeMeshLocalPrefix = 8;
inline constexpr size_t kSizePSKc            = 16;

/**
 * This class provides methods to manipulate Thread operational dataset.
 *
 */
class OperationalDataset
{
public:
    /**
     * This method initializes the dataset with the given dataset.
     *
     * @param[in]   aData       Thread Operational dataset in octects.
     *
     * @retval CHIP_NO_ERROR                Successfully initialized the dataset.
     * @retval CHIP_ERROR_INVALID_ARGUMENT  The dataset length @p aLength is too long or @p data is corrupted.
     */
    CHIP_ERROR Init(ByteSpan aData);

    /**
     * This method retrieves Thread active timestamp from the dataset.
     *
     * @param[out]  aActiveTimestamp    A reference to receive the active timestamp.
     *
     * @retval CHIP_NO_ERROR                    Successfully retrieved the active timestamp.
     * @retval CHIP_ERROR_TLV_TAG_NOT_FOUND     Thread active timestamp is not present in the dataset.
     * @retval CHIP_ERROR_INVALID_TLV_ELEMENT   If the TLV element is invalid.
     */
    CHIP_ERROR GetActiveTimestamp(uint64_t & aActiveTimestamp) const;

    /**
     * This method sets Thread active timestamp to the dataset.
     *
     * @param[in]   aActiveTimestamp    The Thread active timestamp.
     *
     * @retval CHIP_NO_ERROR           Successfully set the active timestamp.
     * @retval CHIP_ERROR_NO_MEMORY    Insufficient memory in the dataset for setting Thread active timestamp.
     */
    CHIP_ERROR SetActiveTimestamp(uint64_t aActiveTimestamp);

    /**
     * This method retrieves Thread channel from the dataset.
     *
     * @param[out]  aChannel    A reference to receive the channel.
     *
     * @retval CHIP_NO_ERROR                    Successfully retrieved the channel.
     * @retval CHIP_ERROR_TLV_TAG_NOT_FOUND     Thread channel is not present in the dataset.
     * @retval CHIP_ERROR_INVALID_TLV_ELEMENT   If the TLV element is invalid.
     */
    CHIP_ERROR GetChannel(uint16_t & aChannel) const;

    /**
     * This method sets Thread channel to the dataset.
     *
     * @param[in]   aChannel    The Thread channel.
     *
     * @retval CHIP_NO_ERROR           Successfully set the channel.
     * @retval CHIP_ERROR_NO_MEMORY    Insufficient memory in the dataset for setting Thread channel.
     */
    CHIP_ERROR SetChannel(uint16_t aChannel);

    /**
     * This method retrieves Thread extended PAN ID from the dataset.
     *
     * @param[out]  aExtendedPanId  A reference to receive the extended PAN ID.
     *
     * @retval CHIP_NO_ERROR                    Successfully retrieved the extended PAN ID.
     * @retval CHIP_ERROR_TLV_TAG_NOT_FOUND     Thread extended PAN ID is not present in the dataset.
     * @retval CHIP_ERROR_INVALID_TLV_ELEMENT   If the TLV element is invalid.
     */
    CHIP_ERROR GetExtendedPanId(uint8_t (&aExtendedPanId)[kSizeExtendedPanId]) const;

    /**
     * This method retrieves the Thread extended PAN ID from the dataset, interpreted as a big endian number.
     * @retval CHIP_NO_ERROR                    Successfully retrieved the extended PAN ID.
     * @retval CHIP_ERROR_TLV_TAG_NOT_FOUND     Thread extended PAN ID is not present in the dataset.
     */
    CHIP_ERROR GetExtendedPanId(uint64_t & extendedPanId) const;

    /**
     * This method returns a const ByteSpan to the extended PAN ID in the dataset.
     * This can be used to pass the extended PAN ID to a cluster command without the use of external memory.
     *
     * Note: The returned span points into storage managed by this class,
     * and must not be dereferenced beyond the lifetime of this object.
     *
     * @param[out]  span  A reference to receive the location of the extended PAN ID.
     *
     * @retval CHIP_NO_ERROR                    Successfully retrieved the extended PAN ID.
     * @retval CHIP_ERROR_TLV_TAG_NOT_FOUND     Thread extended PAN ID is not present in the dataset.
     * @retval CHIP_ERROR_INVALID_TLV_ELEMENT   If the TLV element is invalid.
     */
    CHIP_ERROR GetExtendedPanIdAsByteSpan(ByteSpan & span) const;

    /**
     * This method sets Thread extended PAN ID to the dataset.
     *
     * @param[in]   aExtendedPanId  The Thread extended PAN ID.
     *
     * @retval CHIP_NO_ERROR           Successfully set the extended PAN ID.
     * @retval CHIP_ERROR_NO_MEMORY    Insufficient memory in the dataset for setting Thread extended PAN ID.
     */
    CHIP_ERROR SetExtendedPanId(const uint8_t (&aExtendedPanId)[kSizeExtendedPanId]);

    /**
     * This method retrieves Thread master key from the dataset.
     *
     * @param[out]  aMasterKey  A reference to receive the master key.
     *
     * @retval CHIP_NO_ERROR                    Successfully retrieved the master key.
     * @retval CHIP_ERROR_TLV_TAG_NOT_FOUND     Thread master key is not present in the dataset.
     * @retval CHIP_ERROR_INVALID_TLV_ELEMENT   If the TLV element is invalid.
     */
    CHIP_ERROR GetMasterKey(uint8_t (&aMasterKey)[kSizeMasterKey]) const;

    /**
     * This method sets Thread master key to the dataset.
     *
     * @param[in]   aMasterKey         The Thread master key.
     *
     * @retval CHIP_NO_ERROR           Successfully set the master key.
     * @retval CHIP_ERROR_NO_MEMORY    Insufficient memory in the dataset for setting Thread master key.
     */
    CHIP_ERROR SetMasterKey(const uint8_t (&aMasterKey)[kSizeMasterKey]);

    /**
     * This method unsets Thread master key to the dataset.
     */
    void UnsetMasterKey(void);

    /**
     * This method retrieves Thread mesh local prefix from the dataset.
     *
     * @param[out]  aMeshLocalPrefix    A reference to receive the mesh local prefix.
     *
     * @retval CHIP_NO_ERROR                    Successfully retrieved the mesh local prefix.
     * @retval CHIP_ERROR_TLV_TAG_NOT_FOUND     Thread mesh local prefix is not present in the dataset.
     * @retval CHIP_ERROR_INVALID_TLV_ELEMENT   If the TLV element is invalid.
     */
    CHIP_ERROR GetMeshLocalPrefix(uint8_t (&aMeshLocalPrefix)[kSizeMeshLocalPrefix]) const;

    /**
     * This method sets Thread mesh local prefix to the dataset.
     *
     * @param[in]   aMeshLocalPrefix   The Thread mesh local prefix.
     *
     * @retval CHIP_NO_ERROR           Successfully set the Thread mesh local prefix.
     * @retval CHIP_ERROR_NO_MEMORY    Insufficient memory in the dataset for setting Thread mesh local prefix.
     */
    CHIP_ERROR SetMeshLocalPrefix(const uint8_t (&aMeshLocalPrefix)[kSizeMeshLocalPrefix]);

    /**
     * This method retrieves Thread network name from the dataset.
     *
     * @param[out]  aNetworkName    A reference to receive the Thread network name.
     *
     * @retval CHIP_NO_ERROR                    Successfully retrieved the network name.
     * @retval CHIP_ERROR_TLV_TAG_NOT_FOUND     Thread network name is not present in the dataset.
     * @retval CHIP_ERROR_INVALID_TLV_ELEMENT   If the TLV element is invalid.
     */
    CHIP_ERROR GetNetworkName(char (&aNetworkName)[kSizeNetworkName + 1]) const;

    /**
     * This method sets Thread network name to the dataset.
     *
     * @param[in]   aNetworkName    The Thread network name.
     *
     * @retval CHIP_NO_ERROR           Successfully set the network name.
     * @retval CHIP_ERROR_NO_MEMORY    Insufficient memory in the dataset for setting Thread network name.
     */
    CHIP_ERROR SetNetworkName(const char * aNetworkName);

    /**
     * This method retrieves Thread PAN ID from the dataset.
     *
     * @param[out]  aPanId  A reference to receive the PAN ID.
     *
     * @retval CHIP_NO_ERROR                    Successfully retrieved the PAN ID.
     * @retval CHIP_ERROR_TLV_TAG_NOT_FOUND     Thread PAN ID is not present in the dataset.
     * @retval CHIP_ERROR_INVALID_TLV_ELEMENT   If the TLV element is invalid.
     */
    CHIP_ERROR GetPanId(uint16_t & aPanId) const;

    /**
     * This method sets Thread PAN ID to the dataset.
     *
     * @param[in]   aPanId  The Thread PAN ID.
     *
     * @retval CHIP_NO_ERROR           Successfully set the PAN ID.
     * @retval CHIP_ERROR_NO_MEMORY    Insufficient memory in the dataset for setting Thread PAN ID.
     */
    CHIP_ERROR SetPanId(uint16_t aPanId);

    /**
     * This method retrieves Thread PSKc from the dataset.
     *
     * @param[out]  aPSKc   A reference to receive the PSKc.
     *
     * @retval CHIP_NO_ERROR                    Successfully retrieved the PSKc.
     * @retval CHIP_ERROR_TLV_TAG_NOT_FOUND     Thread PSKc is not present in the dataset.
     * @retval CHIP_ERROR_INVALID_TLV_ELEMENT   If the TLV element is invalid.
     */
    CHIP_ERROR GetPSKc(uint8_t (&aPSKc)[kSizePSKc]) const;

    /**
     * This method sets Thread PSKc to the dataset.
     *
     * @param[in]   aPSKc   The Thread PSKc.
     *
     * @retval CHIP_NO_ERROR           Successfully set the PSKc.
     * @retval CHIP_ERROR_NO_MEMORY    Insufficient memory in the dataset for setting Thread PSKc.
     */
    CHIP_ERROR SetPSKc(const uint8_t (&aPSKc)[kSizePSKc]);

    /**
     * This method unsets Thread PSKc to the dataset.
     */
    void UnsetPSKc(void);

    /**
     * Returns ByteSpan pointing to the channel mask within the dataset.
     *
     * Note: The returned span points into storage managed by this class,
     * and must not be dereferenced beyond the lifetime of this object.
     *
     * @retval CHIP_NO_ERROR on success.
     * @retval CHIP_ERROR_TLV_TAG_NOT_FOUND if the channel mask is not present in the dataset.
     * @retval CHIP_ERROR_INVALID_TLV_ELEMENT if the TLV element is invalid.
     */
    CHIP_ERROR GetChannelMask(ByteSpan & aChannelMask) const;

    /**
     * This method sets the channel mask within the dataset.
     *
     * @retval CHIP_NO_ERROR on success.
     * @retval CHIP_ERROR_NO_MEMORY if there is insufficient space within the dataset.
     */
    CHIP_ERROR SetChannelMask(ByteSpan aChannelMask);

    /**
     * Retrieves the security policy from the dataset.
     *
     * @retval CHIP_NO_ERROR on success.
     * @retval CHIP_ERROR_TLV_TAG_NOT_FOUND if no security policy is present in the dataset.
     * @retval CHIP_ERROR_INVALID_TLV_ELEMENT if the TLV element is invalid.
     */
    CHIP_ERROR GetSecurityPolicy(uint32_t & aSecurityPolicy) const;

    /**
     * This method sets the security policy within the dataset.
     *
     * @retval CHIP_NO_ERROR on success.
     * @retval CHIP_ERROR_NO_MEMORY if there is insufficient space within the dataset.
     */
    CHIP_ERROR SetSecurityPolicy(uint32_t aSecurityPolicy);

    /**
     * Retrieves the delay timer from the dataset.
     *
     * @retval CHIP_NO_ERROR on success.
     * @retval CHIP_ERROR_TLV_TAG_NOT_FOUND if no security policy is present in the dataset.
     * @retval CHIP_ERROR_INVALID_TLV_ELEMENT if the TLV element is invalid.
     */
    CHIP_ERROR GetDelayTimer(uint32_t & aDelayMillis) const;

    /**
     * This method sets the delay timer within the dataset.
     *
     * @retval CHIP_NO_ERROR on success.
     * @retval CHIP_ERROR_NO_MEMORY if there is insufficient space within the dataset.
     */
    CHIP_ERROR SetDelayTimer(uint32_t aDelayMillis);

    /**
     * This method clears all data stored in the dataset.
     */
    void Clear(void) { mLength = 0; }

    /**
     * This method checks if the dataset is ready for creating Thread network.
     */
    bool IsCommissioned(void) const;

    /**
     * This method checks if the dataset is empty.
     */
    bool IsEmpty() const { return mLength == 0; }

    /**
     * This method checks whether @p aData is formatted as ThreadTLVs.
     *
     * @note This method doesn't verify ThreadTLV values are valid.
     */
    static bool IsValid(ByteSpan aData);

    ByteSpan AsByteSpan(void) const { return ByteSpan(mData, mLength); }

private:
    ThreadTLV * Locate(uint8_t aType)
    {
        return const_cast<ThreadTLV *>(const_cast<const OperationalDataset *>(this)->Locate(aType));
    }
    const ThreadTLV * Locate(uint8_t aType) const;
    const ThreadTLV & Begin(void) const { return *reinterpret_cast<const ThreadTLV *>(&mData[0]); };
    ThreadTLV & Begin(void) { return const_cast<ThreadTLV &>(const_cast<const OperationalDataset *>(this)->Begin()); }
    const ThreadTLV & End(void) const { return *reinterpret_cast<const ThreadTLV *>(&mData[mLength]); };
    ThreadTLV & End(void) { return const_cast<ThreadTLV &>(const_cast<const OperationalDataset *>(this)->End()); }
    void Remove(uint8_t aType);
    void Remove(ThreadTLV & aTlv);
    ThreadTLV * MakeRoom(uint8_t aType, size_t aSize);
    bool Has(uint8_t aType) const { return Locate(aType) != nullptr; }

    uint8_t mData[kSizeOperationalDataset];
    uint8_t mLength = 0;
};

} // namespace Thread
} // namespace chip
