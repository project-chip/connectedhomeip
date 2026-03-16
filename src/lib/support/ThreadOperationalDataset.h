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

/// The maximum size of a Thread operational dataset.
inline constexpr size_t kSizeOperationalDataset = 254;

inline constexpr size_t kSizeNetworkName     = 16;
inline constexpr size_t kSizeExtendedPanId   = 8;
inline constexpr size_t kSizeMasterKey       = 16;
inline constexpr size_t kSizeMeshLocalPrefix = 8;
inline constexpr size_t kSizePSKc            = 16;

class OperationalDataset;

/**
 * This class provides a read-only view of a Thread operational dataset.
 * The underlying data is not owned by this class, and must remain valid for the lifetime of the view.
 */
class OperationalDatasetView
{
public:
    OperationalDatasetView() = default;

    /**
     * Initializes the dataset view with the given data.
     * The data itself is not copied, so it must remain valid for the lifetime of the view.
     *
     * @param[in]   aData       The data to interpret as a Thread operational dataset.
     *
     * @retval CHIP_NO_ERROR                Successfully initialized the dataset.
     * @retval CHIP_ERROR_INVALID_ARGUMENT  The dataset length @p aLength is too long or @p data is corrupted.
     */
    CHIP_ERROR Init(ByteSpan aData);

    /**
     * Returns the ByteSpan underlying this view.
     */
    ByteSpan AsByteSpan() const { return mData; }

    /**
     * Retrieves the Active Timestamp from the dataset.
     *
     * @param[out]  aActiveTimestamp    A reference to receive the active timestamp.
     *
     * @retval CHIP_NO_ERROR                    Successfully retrieved the active timestamp.
     * @retval CHIP_ERROR_TLV_TAG_NOT_FOUND     Thread active timestamp is not present in the dataset.
     * @retval CHIP_ERROR_INVALID_TLV_ELEMENT   If the TLV element is invalid.
     */
    CHIP_ERROR GetActiveTimestamp(uint64_t & aActiveTimestamp) const;

    /**
     * Retrieves the channel number from the dataset.
     *
     * Note that the underlying TLV consists of a 1 byte Channel Page, and a 2 byte Channel Number.
     * The Channel Page is not returned, as zero is currently the only valid value.
     * A non-zero Channel Page is treated as a CHIP_ERROR_INVALID_TLV_ELEMENT error
     *
     * @param[out]  aChannel    A reference to receive the channel.
     *
     * @retval CHIP_NO_ERROR                    Successfully retrieved the channel.
     * @retval CHIP_ERROR_TLV_TAG_NOT_FOUND     Thread channel is not present in the dataset.
     * @retval CHIP_ERROR_INVALID_TLV_ELEMENT   If the TLV element is invalid.
     */
    CHIP_ERROR GetChannel(uint16_t & aChannel) const;

    /**
     * Retrieves the Extended PAN ID from the dataset.
     *
     * @param[out]  aExtendedPanId  A reference to receive the extended PAN ID.
     *
     * @retval CHIP_NO_ERROR                    Successfully retrieved the extended PAN ID.
     * @retval CHIP_ERROR_TLV_TAG_NOT_FOUND     Thread extended PAN ID is not present in the dataset.
     * @retval CHIP_ERROR_INVALID_TLV_ELEMENT   If the TLV element is invalid.
     */
    CHIP_ERROR GetExtendedPanId(uint8_t (&aExtendedPanId)[kSizeExtendedPanId]) const;

    /**
     * Retrieves the Extended PAN ID from the dataset, interpreted as a big endian number.
     * @retval CHIP_NO_ERROR                    Successfully retrieved the extended PAN ID.
     * @retval CHIP_ERROR_TLV_TAG_NOT_FOUND     Thread extended PAN ID is not present in the dataset.
     */
    CHIP_ERROR GetExtendedPanId(uint64_t & extendedPanId) const;

    /**
     * Retrieves a ByteSpan pointing to the Extended PAN ID within the dataset.
     * This can be used to pass the extended PAN ID to a cluster command without the use of external memory.
     *
     * Note: The returned span points must not be dereferenced beyond the lifetime of this object.
     *
     * @param[out]  span  A reference to receive the location of the extended PAN ID.
     *
     * @retval CHIP_NO_ERROR                    Successfully retrieved the extended PAN ID.
     * @retval CHIP_ERROR_TLV_TAG_NOT_FOUND     Thread extended PAN ID is not present in the dataset.
     * @retval CHIP_ERROR_INVALID_TLV_ELEMENT   If the TLV element is invalid.
     */
    CHIP_ERROR GetExtendedPanIdAsByteSpan(ByteSpan & span) const;

    /**
     * Retrieves the Master Key from the dataset.
     *
     * @param[out]  aMasterKey  A reference to receive the master key.
     *
     * @retval CHIP_NO_ERROR                    Successfully retrieved the master key.
     * @retval CHIP_ERROR_TLV_TAG_NOT_FOUND     Thread master key is not present in the dataset.
     * @retval CHIP_ERROR_INVALID_TLV_ELEMENT   If the TLV element is invalid.
     */
    CHIP_ERROR GetMasterKey(uint8_t (&aMasterKey)[kSizeMasterKey]) const;

    /**
     * Retrieves the Mesh Local Prefix from the dataset.
     *
     * @param[out]  aMeshLocalPrefix    A reference to receive the mesh local prefix.
     *
     * @retval CHIP_NO_ERROR                    Successfully retrieved the mesh local prefix.
     * @retval CHIP_ERROR_TLV_TAG_NOT_FOUND     Thread mesh local prefix is not present in the dataset.
     * @retval CHIP_ERROR_INVALID_TLV_ELEMENT   If the TLV element is invalid.
     */
    CHIP_ERROR GetMeshLocalPrefix(uint8_t (&aMeshLocalPrefix)[kSizeMeshLocalPrefix]) const;

    /**
     * Retrieves the Network Name from the dataset.
     *
     * @param[out]  aNetworkName    A reference to receive the Thread network name.
     *
     * @retval CHIP_NO_ERROR                    Successfully retrieved the network name.
     * @retval CHIP_ERROR_TLV_TAG_NOT_FOUND     Thread network name is not present in the dataset.
     * @retval CHIP_ERROR_INVALID_TLV_ELEMENT   If the TLV element is invalid.
     */
    CHIP_ERROR GetNetworkName(char (&aNetworkName)[kSizeNetworkName + 1]) const;

    /**
     * Retrieves the PAN ID from the dataset.
     *
     * @param[out]  aPanId  A reference to receive the PAN ID.
     *
     * @retval CHIP_NO_ERROR                    Successfully retrieved the PAN ID.
     * @retval CHIP_ERROR_TLV_TAG_NOT_FOUND     Thread PAN ID is not present in the dataset.
     * @retval CHIP_ERROR_INVALID_TLV_ELEMENT   If the TLV element is invalid.
     */
    CHIP_ERROR GetPanId(uint16_t & aPanId) const;

    /**
     * Retrieves the Pre-Shared Key for the Commissioner (PSKc) from the dataset.
     *
     * @param[out]  aPSKc   A reference to receive the PSKc.
     *
     * @retval CHIP_NO_ERROR                    Successfully retrieved the PSKc.
     * @retval CHIP_ERROR_TLV_TAG_NOT_FOUND     Thread PSKc is not present in the dataset.
     * @retval CHIP_ERROR_INVALID_TLV_ELEMENT   If the TLV element is invalid.
     */
    CHIP_ERROR GetPSKc(uint8_t (&aPSKc)[kSizePSKc]) const;

    /**
     * Retrieves a ByteSpan pointing to the Channel Mask within the dataset.
     *
     * Note: The returned span must not be dereferenced beyond the lifetime of this object.
     *
     * @retval CHIP_NO_ERROR on success.
     * @retval CHIP_ERROR_TLV_TAG_NOT_FOUND if the channel mask is not present in the dataset.
     * @retval CHIP_ERROR_INVALID_TLV_ELEMENT if the TLV element is invalid.
     */
    CHIP_ERROR GetChannelMask(ByteSpan & aChannelMask) const;

    /**
     * Retrieves the Security Policy from the dataset.
     *
     * @retval CHIP_NO_ERROR on success.
     * @retval CHIP_ERROR_TLV_TAG_NOT_FOUND if no security policy is present in the dataset.
     * @retval CHIP_ERROR_INVALID_TLV_ELEMENT if the TLV element is invalid.
     */
    CHIP_ERROR GetSecurityPolicy(uint32_t & aSecurityPolicy) const;

    /**
     * Retrieves the Delay Timer from the dataset.
     *
     * @retval CHIP_NO_ERROR on success.
     * @retval CHIP_ERROR_TLV_TAG_NOT_FOUND if no security policy is present in the dataset.
     * @retval CHIP_ERROR_INVALID_TLV_ELEMENT if the TLV element is invalid.
     */
    CHIP_ERROR GetDelayTimer(uint32_t & aDelayMillis) const;

    /**
     * Returns true if the dataset contains the required TLV elements for creating a Thread network.
     * The required elements are: PAN ID, Extended PAN ID, Channel, and Master Key.
     */
    bool IsCommissioned() const;

    /**
     * This method checks if the dataset is empty.
     */
    bool IsEmpty() const { return mData.empty(); }

    /**
     * This method checks whether @p aData contains a valid sequence of Thread TLV elements.
     *
     * @note This method only verifies the overall TLV format, not the correctness of individual TLV elements.
     */
    static bool IsValid(ByteSpan aData);

private:
    friend class OperationalDataset;

    ByteSpan mData;

    OperationalDatasetView(uint8_t * buffer) : mData(ByteSpan(buffer, 0)) {}

    const ThreadTLV * Locate(uint8_t aType) const;
    bool Has(uint8_t aType) const { return Locate(aType) != nullptr; }
};

/**
 * This class provides methods to manipulate a Thread operational dataset.
 * It maintains an internal buffer sized to accommodate the maximum possible dataset size.
 */
class OperationalDataset : public OperationalDatasetView
{
public:
    OperationalDataset() : OperationalDatasetView(mBuffer) {}

    // Delegate copy construction and assignment to the overloads taking an OperationalDatasetView.
    OperationalDataset(const OperationalDataset & other) : OperationalDataset(static_cast<const OperationalDatasetView &>(other)) {}
    OperationalDataset & operator=(const OperationalDataset & other)
    {
        return *this = static_cast<const OperationalDatasetView &>(other);
    }

    OperationalDataset(const OperationalDatasetView & view) : OperationalDatasetView(view) { CopyData(); }
    OperationalDataset & operator=(const OperationalDatasetView & view)
    {
        static_cast<OperationalDatasetView &>(*this) = view;
        CopyData();
        return *this;
    }

    /**
     * Initializes the dataset by copying the provided data into an internal buffer.
     *
     * @param[in]   aData       The data to interpret as a Thread operational dataset.
     *
     * @retval CHIP_NO_ERROR                Successfully initialized the dataset.
     * @retval CHIP_ERROR_INVALID_ARGUMENT  The dataset length @p aLength is too long or @p data is corrupted.
     */
    CHIP_ERROR Init(ByteSpan aData);

    /**
     * Sets the Active Timestamp in the dataset.
     *
     * @param[in]   aActiveTimestamp    The Thread active timestamp.
     *
     * @retval CHIP_NO_ERROR           Successfully set the active timestamp.
     * @retval CHIP_ERROR_NO_MEMORY    Insufficient memory in the dataset for setting Thread active timestamp.
     */
    CHIP_ERROR SetActiveTimestamp(uint64_t aActiveTimestamp);

    /**
     * Sets the Channel Number in the dataset.
     *
     * Note that the underlying TLV consists of a 1 byte Channel Page, and a 2 byte Channel Number.
     * The Channel Page is always set to zero, as it is currently the only valid value.
     *
     * @param[in]   aChannel    The Thread channel.
     *
     * @retval CHIP_NO_ERROR           Successfully set the channel.
     * @retval CHIP_ERROR_NO_MEMORY    Insufficient memory in the dataset for setting Thread channel.
     */
    CHIP_ERROR SetChannel(uint16_t aChannel);

    /**
     * Sets the Extended PAN ID in the dataset.
     *
     * @param[in]   aExtendedPanId  The Thread extended PAN ID.
     *
     * @retval CHIP_NO_ERROR           Successfully set the extended PAN ID.
     * @retval CHIP_ERROR_NO_MEMORY    Insufficient memory in the dataset for setting Thread extended PAN ID.
     */
    CHIP_ERROR SetExtendedPanId(const uint8_t (&aExtendedPanId)[kSizeExtendedPanId]);

    /**
     * Sets the Master Key in the dataset.
     *
     * @param[in]   aMasterKey         The Thread master key.
     *
     * @retval CHIP_NO_ERROR           Successfully set the master key.
     * @retval CHIP_ERROR_NO_MEMORY    Insufficient memory in the dataset for setting Thread master key.
     */
    CHIP_ERROR SetMasterKey(const uint8_t (&aMasterKey)[kSizeMasterKey]);

    /**
     * Removes the Master Key from the dataset.
     */
    void UnsetMasterKey();

    /**
     * Sets the Mesh Local Prefix in the dataset.
     *
     * @param[in]   aMeshLocalPrefix   The Thread mesh local prefix.
     *
     * @retval CHIP_NO_ERROR           Successfully set the Thread mesh local prefix.
     * @retval CHIP_ERROR_NO_MEMORY    Insufficient memory in the dataset for setting Thread mesh local prefix.
     */
    CHIP_ERROR SetMeshLocalPrefix(const uint8_t (&aMeshLocalPrefix)[kSizeMeshLocalPrefix]);

    /**
     * Sets the Network Name in the dataset. The name must be a non-empty string
     * with a maximum length of 16 characters (kSizeNetworkName).
     *
     * @param[in]   aNetworkName    The Thread network name.
     *
     * @retval CHIP_NO_ERROR           Successfully set the network name.
     * @retval CHIP_ERROR_NO_MEMORY    Insufficient memory in the dataset for setting Thread network name.
     */
    CHIP_ERROR SetNetworkName(const char * aNetworkName);

    /**
     * Sets the PAN ID in the dataset.
     *
     * @param[in]   aPanId  The Thread PAN ID.
     *
     * @retval CHIP_NO_ERROR           Successfully set the PAN ID.
     * @retval CHIP_ERROR_NO_MEMORY    Insufficient memory in the dataset for setting Thread PAN ID.
     */
    CHIP_ERROR SetPanId(uint16_t aPanId);

    /**
     * Sets the Pre-Shared Key for the Commissioner (PSKc) in the dataset.
     *
     * @param[in]   aPSKc   The Thread PSKc.
     *
     * @retval CHIP_NO_ERROR           Successfully set the PSKc.
     * @retval CHIP_ERROR_NO_MEMORY    Insufficient memory in the dataset for setting Thread PSKc.
     */
    CHIP_ERROR SetPSKc(const uint8_t (&aPSKc)[kSizePSKc]);

    /**
     * Removes the Pre-Shared Key for the Commissioner (PSKc) from the dataset.
     */
    void UnsetPSKc();

    /**
     * Sets the Channel Mask in the dataset. This value is a non-empty sequence of sub-TLVs as defined in the Thread specification.
     *
     * @retval CHIP_NO_ERROR on success.
     * @retval CHIP_ERROR_NO_MEMORY if there is insufficient space within the dataset.
     */
    CHIP_ERROR SetChannelMask(ByteSpan aChannelMask);

    /**
     * Sets the Security Policy in the dataset.
     *
     * @retval CHIP_NO_ERROR on success.
     * @retval CHIP_ERROR_NO_MEMORY if there is insufficient space within the dataset.
     */
    CHIP_ERROR SetSecurityPolicy(uint32_t aSecurityPolicy);

    /**
     * Sets the Delay Timer value in the dataset.
     *
     * @retval CHIP_NO_ERROR on success.
     * @retval CHIP_ERROR_NO_MEMORY if there is insufficient space within the dataset.
     */
    CHIP_ERROR SetDelayTimer(uint32_t aDelayMillis);

    /**
     * This method clears all data stored in the dataset.
     */
    void Clear() { mData = ByteSpan(mBuffer, 0); }

    /**
     * Returns a ByteSpan view of the current state of the dataset.
     * The byte span is only valid as long the OperationalDataset object is alive and not modified.
     */
    ByteSpan AsByteSpan() const { return this->OperationalDatasetView::AsByteSpan(); }

private:
    uint8_t mBuffer[kSizeOperationalDataset];

    void CopyData();
    void CopyDataIfNecessary();
    void Remove(uint8_t aType);
    void Remove(ThreadTLV * aTlv);
    ThreadTLV * InsertOrReplace(uint8_t aType, size_t aValueSize);
};

} // namespace Thread
} // namespace chip
