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

#include <lib/support/ThreadOperationalDataset.h>

#include <lib/core/CHIPEncoding.h>

#include <cassert>
#include <cstring>

namespace chip {
namespace Thread {

/**
 * The Thread specification defines two TLV element formats: a "base" format and an "extended" format.
 * The base format is used for TLV elements with a length of 0-254 bytes and is laid out as follows:
 *
 * +---------+---------+-------------------------+
 * | uint8_t | uint8_t | network byte order data |
 * | 1 byte  | 1 byte  | n byte (0 <= n < 255)   |
 * +---------+---------+-------------------------+
 * |  Type   | Length  | Value                   |
 * +---------+---------+-------------------------+
 *
 * A length values of 0xff (255) is an "escape" value and indicates that the element uses the "extended"
 * format, where a two-byte length field follows the 0xff escape byte.
 *
 * Only the base format is allowed in Thread Operational Datasets, so only this format is supported here.
 */
class ThreadTLV final
{
public:
    static constexpr uint8_t kLengthEscape = 0xff;

    enum : uint8_t
    {
        kChannel         = 0,
        kPanId           = 1,
        kExtendedPanId   = 2,
        kNetworkName     = 3,
        kPSKc            = 4,
        kMasterKey       = 5,
        kMeshLocalPrefix = 7,
        kSecurityPolicy  = 12,
        kActiveTimestamp = 14,
        kDelayTimer      = 52,
        kChannelMask     = 53,
    };

    size_t GetSize() const
    {
        static_assert(sizeof(*this) == 2, "ThreadTLV header should be 2 bytes (type, length)");
        return sizeof(*this) + GetLength();
    }

    uint8_t GetType() const { return mType; }

    void SetType(uint8_t aType) { mType = aType; }

    uint8_t GetLength() const { return mLength; }

    void SetLength(size_t aLength)
    {
        assert(aLength < kLengthEscape);
        mLength = static_cast<uint8_t>(aLength);
    }

    const uint8_t * GetValue() const
    {
        assert(mLength != kLengthEscape);
        return reinterpret_cast<const uint8_t *>(this) + sizeof(*this);
    }

    uint8_t * GetValue() { return const_cast<uint8_t *>(const_cast<const ThreadTLV *>(this)->GetValue()); }

    ByteSpan GetValueAsSpan() const { return ByteSpan(GetValue(), GetLength()); }

    void Get64(uint64_t & aValue) const
    {
        assert(GetLength() == sizeof(aValue));
        aValue = Encoding::BigEndian::Get64(GetValue());
    }

    void Get32(uint32_t & aValue) const
    {
        assert(GetLength() == sizeof(aValue));
        aValue = Encoding::BigEndian::Get32(GetValue());
    }

    void Get16(uint16_t & aValue) const
    {
        assert(GetLength() == sizeof(aValue));
        aValue = Encoding::BigEndian::Get16(GetValue());
    }

    void Set64(uint64_t aValue)
    {
        assert(GetLength() == sizeof(aValue));
        Encoding::BigEndian::Put64(GetValue(), aValue);
    }

    void Set32(uint32_t aValue)
    {
        assert(GetLength() == sizeof(aValue));
        Encoding::BigEndian::Put32(GetValue(), aValue);
    }

    void Set16(uint16_t aValue)
    {
        assert(GetLength() == sizeof(aValue));
        Encoding::BigEndian::Put16(GetValue(), aValue);
    }

    void SetValue(const void * aValue, size_t aLength)
    {
        assert(GetLength() == aLength);
        memcpy(GetValue(), aValue, aLength);
    }

    void SetValue(const ByteSpan & aValue) { SetValue(aValue.data(), aValue.size()); }

    const ThreadTLV * GetNext() const
    {
        static_assert(alignof(ThreadTLV) == 1, "Wrong alignment for ThreadTLV header");
        return reinterpret_cast<const ThreadTLV *>(static_cast<const uint8_t *>(GetValue()) + GetLength());
    }

    ThreadTLV * GetNext() { return reinterpret_cast<ThreadTLV *>(static_cast<uint8_t *>(GetValue()) + GetLength()); }

private:
    uint8_t mType;
    uint8_t mLength;
};

/// OperationalDatasetView

bool OperationalDatasetView::IsValid(ByteSpan aData)
{
    VerifyOrReturnValue(aData.size() <= kSizeOperationalDataset, false);

    const ThreadTLV * tlv = reinterpret_cast<const ThreadTLV *>(aData.begin());
    const ThreadTLV * end = reinterpret_cast<const ThreadTLV *>(aData.end());
    while (tlv != end)
    {
        VerifyOrReturnValue(tlv + 1 <= end, false);                               // out of bounds
        VerifyOrReturnValue(tlv->GetLength() != ThreadTLV::kLengthEscape, false); // not allowed in a dataset TLV
        tlv = tlv->GetNext();
    }
    return true;
}

CHIP_ERROR OperationalDatasetView::Init(ByteSpan aData)
{
    VerifyOrReturnError(IsValid(aData), CHIP_ERROR_INVALID_ARGUMENT);
    mData = aData;
    return CHIP_NO_ERROR;
}

const ThreadTLV * OperationalDatasetView::Locate(uint8_t aType) const
{
    const ThreadTLV * tlv = reinterpret_cast<const ThreadTLV *>(mData.begin());
    const ThreadTLV * end = reinterpret_cast<const ThreadTLV *>(mData.end());
    while (tlv < end)
    {
        if (tlv->GetType() == aType)
        {
            return tlv;
        }
        tlv = tlv->GetNext();
    }
    return nullptr;
}

bool OperationalDatasetView::IsCommissioned() const
{
    return Has(ThreadTLV::kPanId) && Has(ThreadTLV::kMasterKey) && Has(ThreadTLV::kExtendedPanId) && Has(ThreadTLV::kChannel);
}

CHIP_ERROR OperationalDatasetView::GetActiveTimestamp(uint64_t & aActiveTimestamp) const
{
    const ThreadTLV * tlv = Locate(ThreadTLV::kActiveTimestamp);
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_TLV_TAG_NOT_FOUND);
    VerifyOrReturnError(tlv->GetLength() == sizeof(aActiveTimestamp), CHIP_ERROR_INVALID_TLV_ELEMENT);
    tlv->Get64(aActiveTimestamp);
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDatasetView::GetChannel(uint16_t & aChannel) const
{
    const ThreadTLV * tlv = Locate(ThreadTLV::kChannel);
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_TLV_TAG_NOT_FOUND);
    VerifyOrReturnError(tlv->GetLength() == 3, CHIP_ERROR_INVALID_TLV_ELEMENT);
    const uint8_t * value = tlv->GetValue();
    VerifyOrReturnError(value[0] == 0, CHIP_ERROR_INVALID_TLV_ELEMENT); // Channel Page must be 0
    aChannel = Encoding::BigEndian::Get16(value + 1);
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDatasetView::GetExtendedPanId(uint8_t (&aExtendedPanId)[kSizeExtendedPanId]) const
{
    ByteSpan extPanIdSpan;
    ReturnErrorOnFailure(GetExtendedPanIdAsByteSpan(extPanIdSpan));
    memcpy(aExtendedPanId, extPanIdSpan.data(), extPanIdSpan.size());
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDatasetView::GetExtendedPanId(uint64_t & extendedPanId) const
{
    ByteSpan extPanIdSpan;
    ReturnErrorOnFailure(GetExtendedPanIdAsByteSpan(extPanIdSpan));
    VerifyOrDie(extPanIdSpan.size() == sizeof(extendedPanId));
    extendedPanId = Encoding::BigEndian::Get64(extPanIdSpan.data());
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDatasetView::GetExtendedPanIdAsByteSpan(ByteSpan & span) const
{
    const ThreadTLV * tlv = Locate(ThreadTLV::kExtendedPanId);
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_TLV_TAG_NOT_FOUND);
    VerifyOrReturnError(tlv->GetLength() == kSizeExtendedPanId, CHIP_ERROR_INVALID_TLV_ELEMENT);
    span = tlv->GetValueAsSpan();
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDatasetView::GetMeshLocalPrefix(uint8_t (&aMeshLocalPrefix)[kSizeMeshLocalPrefix]) const
{
    const ThreadTLV * tlv = Locate(ThreadTLV::kMeshLocalPrefix);
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_TLV_TAG_NOT_FOUND);
    VerifyOrReturnError(tlv->GetLength() == sizeof(aMeshLocalPrefix), CHIP_ERROR_INVALID_TLV_ELEMENT);
    memcpy(aMeshLocalPrefix, tlv->GetValue(), sizeof(aMeshLocalPrefix));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDatasetView::GetMasterKey(uint8_t (&aMasterKey)[kSizeMasterKey]) const
{
    const ThreadTLV * tlv = Locate(ThreadTLV::kMasterKey);
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_TLV_TAG_NOT_FOUND);
    VerifyOrReturnError(tlv->GetLength() == sizeof(aMasterKey), CHIP_ERROR_INVALID_TLV_ELEMENT);
    memcpy(aMasterKey, tlv->GetValue(), sizeof(aMasterKey));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDatasetView::GetNetworkName(char (&aNetworkName)[kSizeNetworkName + 1]) const
{
    const ThreadTLV * tlv = Locate(ThreadTLV::kNetworkName);
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_TLV_TAG_NOT_FOUND);
    VerifyOrReturnError(tlv->GetLength() <= kSizeNetworkName, CHIP_ERROR_INVALID_TLV_ELEMENT);
    memcpy(aNetworkName, tlv->GetValue(), tlv->GetLength());
    aNetworkName[tlv->GetLength()] = '\0';
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDatasetView::GetPanId(uint16_t & aPanId) const
{
    const ThreadTLV * tlv = Locate(ThreadTLV::kPanId);
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_TLV_TAG_NOT_FOUND);
    VerifyOrReturnError(tlv->GetLength() == sizeof(aPanId), CHIP_ERROR_INVALID_TLV_ELEMENT);
    tlv->Get16(aPanId);
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDatasetView::GetPSKc(uint8_t (&aPSKc)[kSizePSKc]) const
{
    const ThreadTLV * tlv = Locate(ThreadTLV::kPSKc);
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_TLV_TAG_NOT_FOUND);
    VerifyOrReturnError(tlv->GetLength() == sizeof(aPSKc), CHIP_ERROR_INVALID_TLV_ELEMENT);
    memcpy(aPSKc, tlv->GetValue(), sizeof(aPSKc));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDatasetView::GetChannelMask(ByteSpan & aChannelMask) const
{
    const ThreadTLV * tlv = Locate(ThreadTLV::kChannelMask);
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_TLV_TAG_NOT_FOUND);
    VerifyOrReturnError(tlv->GetLength() > 0, CHIP_ERROR_INVALID_TLV_ELEMENT);
    aChannelMask = tlv->GetValueAsSpan();
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDatasetView::GetSecurityPolicy(uint32_t & aSecurityPolicy) const
{
    const ThreadTLV * tlv = Locate(ThreadTLV::kSecurityPolicy);
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_TLV_TAG_NOT_FOUND);
    VerifyOrReturnError(tlv->GetLength() == sizeof(aSecurityPolicy), CHIP_ERROR_INVALID_TLV_ELEMENT);
    tlv->Get32(aSecurityPolicy);
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDatasetView::GetDelayTimer(uint32_t & aDelayMillis) const
{
    const ThreadTLV * tlv = Locate(ThreadTLV::kDelayTimer);
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_TLV_TAG_NOT_FOUND);
    VerifyOrReturnError(tlv->GetLength() == sizeof(aDelayMillis), CHIP_ERROR_INVALID_TLV_ELEMENT);
    tlv->Get32(aDelayMillis);
    return CHIP_NO_ERROR;
}

/// OperationalDataset

inline constexpr size_t kMaxDatasetElementLength = kSizeOperationalDataset - sizeof(ThreadTLV);

CHIP_ERROR OperationalDataset::Init(ByteSpan aData)
{
    VerifyOrReturnError(IsValid(aData), CHIP_ERROR_INVALID_ARGUMENT);
    // Use memmove because aData could be a sub-span of AsByteSpan()
    memmove(mBuffer, aData.data(), aData.size());
    mData = ByteSpan(mBuffer, aData.size());
    return CHIP_NO_ERROR;
}

void OperationalDataset::CopyDataIfNecessary()
{
    // It's possible that mData points into an external buffer if someone has
    // called OperationalDatasetView::Init() instead of our copying version.
    if (mData.data() != mBuffer)
    {
        CopyData();
    }
}

void OperationalDataset::CopyData()
{
    memmove(mBuffer, mData.data(), mData.size());
    mData = ByteSpan(mBuffer, mData.size());
}

void OperationalDataset::Remove(ThreadTLV * tlv)
{
    size_t size      = tlv->GetSize();
    ThreadTLV * next = tlv->GetNext();
    memmove(tlv, next, static_cast<size_t>(mData.end() - reinterpret_cast<uint8_t *>(next)));
    mData = ByteSpan(mData.data(), mData.size() - size);
}

void OperationalDataset::Remove(uint8_t aType)
{
    CopyDataIfNecessary();
    ThreadTLV * tlv = const_cast<ThreadTLV *>(Locate(aType));
    if (tlv != nullptr)
    {
        Remove(tlv);
    }
}

// Inserts a TLV of the specified type and length into the dataset, replacing an existing TLV
// of the same type if one exists. Returns nullptr if there is not enough space.
ThreadTLV * OperationalDataset::InsertOrReplace(uint8_t aType, size_t aValueSize)
{
    assert(aValueSize <= kMaxDatasetElementLength); // callers check this or a tighter limit
    CopyDataIfNecessary();
    ThreadTLV * tlv = const_cast<ThreadTLV *>(Locate(aType));
    if (tlv != nullptr)
    {
        size_t tlvLength = tlv->GetLength();
        VerifyOrReturnValue(aValueSize != tlvLength, tlv); // re-use in place if same size
        VerifyOrReturnValue(aValueSize < tlvLength || mData.size() + aValueSize - tlvLength <= sizeof(mBuffer), nullptr);
        Remove(tlv); // we could grow or shrink in place instead, but this is simpler
    }
    else
    {
        VerifyOrReturnValue(mData.size() + sizeof(ThreadTLV) + aValueSize <= sizeof(mBuffer), nullptr);
    }

    tlv = reinterpret_cast<ThreadTLV *>(mBuffer + mData.size());
    tlv->SetType(aType);
    tlv->SetLength(static_cast<uint8_t>(aValueSize));
    mData = ByteSpan(mBuffer, mData.size() + tlv->GetSize());
    return tlv;
}

CHIP_ERROR OperationalDataset::SetActiveTimestamp(uint64_t aActiveTimestamp)
{
    static_assert(sizeof(aActiveTimestamp) <= kMaxDatasetElementLength);
    ThreadTLV * tlv = InsertOrReplace(ThreadTLV::kActiveTimestamp, sizeof(aActiveTimestamp));
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_NO_MEMORY);
    tlv->Set64(aActiveTimestamp);
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::SetChannel(uint16_t aChannel)
{
    uint8_t value[3] = { 0 }; // Channel Page is always 0
    Encoding::BigEndian::Put16(value + 1, aChannel);
    static_assert(sizeof(value) <= kMaxDatasetElementLength);
    ThreadTLV * tlv = InsertOrReplace(ThreadTLV::kChannel, sizeof(value));
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_NO_MEMORY);
    tlv->SetValue(value, sizeof(value));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::SetExtendedPanId(const uint8_t (&aExtendedPanId)[kSizeExtendedPanId])
{
    static_assert(sizeof(aExtendedPanId) <= kMaxDatasetElementLength);
    ThreadTLV * tlv = InsertOrReplace(ThreadTLV::kExtendedPanId, sizeof(aExtendedPanId));
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_NO_MEMORY);
    tlv->SetValue(aExtendedPanId, sizeof(aExtendedPanId));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::SetMasterKey(const uint8_t (&aMasterKey)[kSizeMasterKey])
{
    static_assert(sizeof(aMasterKey) <= kMaxDatasetElementLength);
    ThreadTLV * tlv = InsertOrReplace(ThreadTLV::kMasterKey, sizeof(aMasterKey));
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_NO_MEMORY);
    tlv->SetValue(aMasterKey, sizeof(aMasterKey));
    return CHIP_NO_ERROR;
}

void OperationalDataset::UnsetMasterKey()
{
    Remove(ThreadTLV::kMasterKey);
}

CHIP_ERROR OperationalDataset::SetMeshLocalPrefix(const uint8_t (&aMeshLocalPrefix)[kSizeMeshLocalPrefix])
{
    static_assert(sizeof(aMeshLocalPrefix) <= kMaxDatasetElementLength);
    ThreadTLV * tlv = InsertOrReplace(ThreadTLV::kMeshLocalPrefix, sizeof(aMeshLocalPrefix));
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_NO_MEMORY);
    tlv->SetValue(aMeshLocalPrefix, sizeof(aMeshLocalPrefix));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::SetNetworkName(const char * aNetworkName)
{
    VerifyOrReturnError(aNetworkName != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    size_t len = strlen(aNetworkName);
    VerifyOrReturnError(0 < len && len <= kSizeNetworkName, CHIP_ERROR_INVALID_STRING_LENGTH);

    static_assert(kSizeNetworkName <= kMaxDatasetElementLength);
    ThreadTLV * tlv = InsertOrReplace(ThreadTLV::kNetworkName, len);
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_NO_MEMORY);
    tlv->SetValue(aNetworkName, len);
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::SetPanId(uint16_t aPanId)
{
    static_assert(sizeof(aPanId) <= kMaxDatasetElementLength);
    ThreadTLV * tlv = InsertOrReplace(ThreadTLV::kPanId, sizeof(aPanId));
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_NO_MEMORY);
    tlv->Set16(aPanId);
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::SetPSKc(const uint8_t (&aPSKc)[kSizePSKc])
{
    static_assert(sizeof(aPSKc) <= kMaxDatasetElementLength);
    ThreadTLV * tlv = InsertOrReplace(ThreadTLV::kPSKc, sizeof(aPSKc));
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_NO_MEMORY);
    tlv->SetValue(aPSKc, sizeof(aPSKc));
    return CHIP_NO_ERROR;
}

void OperationalDataset::UnsetPSKc()
{
    Remove(ThreadTLV::kPSKc);
}

CHIP_ERROR OperationalDataset::SetChannelMask(ByteSpan aChannelMask)
{
    VerifyOrReturnError(0 < aChannelMask.size() && aChannelMask.size() <= kMaxDatasetElementLength, CHIP_ERROR_INVALID_ARGUMENT);
    ThreadTLV * tlv = InsertOrReplace(ThreadTLV::kChannelMask, aChannelMask.size());
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_NO_MEMORY);
    tlv->SetValue(aChannelMask);
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::SetSecurityPolicy(uint32_t aSecurityPolicy)
{
    static_assert(sizeof(aSecurityPolicy) <= kMaxDatasetElementLength);
    ThreadTLV * tlv = InsertOrReplace(ThreadTLV::kSecurityPolicy, sizeof(aSecurityPolicy));
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_NO_MEMORY);
    tlv->Set32(aSecurityPolicy);
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::SetDelayTimer(uint32_t aDelayMillis)
{
    static_assert(sizeof(aDelayMillis) <= kMaxDatasetElementLength);
    ThreadTLV * tlv = InsertOrReplace(ThreadTLV::kDelayTimer, sizeof(aDelayMillis));
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_NO_MEMORY);
    tlv->Set32(aDelayMillis);
    return CHIP_NO_ERROR;
}

} // namespace Thread
} // namespace chip
