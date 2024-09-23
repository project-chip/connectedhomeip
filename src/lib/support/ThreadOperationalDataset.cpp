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
 * Thread Operational Dataset TLV is defined in Thread Specification as the following format:
 *
 * +---------+---------+-------------------------+
 * | uint8_t | uint8_t | network byte order data |
 * | 1 byte  | 1 byte  | n byte (0 <= n < 255)   |
 * +---------+---------+-------------------------+
 * |  Type   | Length  | Value                   |
 * +---------+---------+-------------------------+
 *
 */
class ThreadTLV final
{
    static constexpr uint8_t kLengthEscape = 0xff; ///< This length value indicates the actual length is of two-bytes length, which
                                                   ///< is not allowed in Thread Operational Dataset TLVs.

public:
    static constexpr uint8_t kMaxLength = kLengthEscape - 1;

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

    uint8_t GetSize() const { return static_cast<uint8_t>(sizeof(*this) + GetLength()); }

    uint8_t GetType() const { return mType; }

    void SetType(uint8_t aType) { mType = aType; }

    uint8_t GetLength() const
    {
        assert(mLength != kLengthEscape);
        return mLength;
    }

    void SetLength(uint8_t aLength)
    {
        assert(mLength != kLengthEscape);
        mLength = aLength;
    }

    const uint8_t * GetValue() const
    {
        assert(mLength != kLengthEscape);

        static_assert(sizeof(*this) == sizeof(ThreadTLV::mType) + sizeof(ThreadTLV::mLength), "Wrong size for ThreadTLV header");

        return reinterpret_cast<const uint8_t *>(this) + sizeof(*this);
    }

    uint8_t * GetValue() { return const_cast<uint8_t *>(const_cast<const ThreadTLV *>(this)->GetValue()); }

    ByteSpan GetValueAsSpan() const { return ByteSpan(static_cast<const uint8_t *>(GetValue()), GetLength()); }

    void Get64(uint64_t & aValue) const
    {
        assert(GetLength() >= sizeof(aValue));
        aValue = Encoding::BigEndian::Get64(GetValue());
    }

    void Get32(uint32_t & aValue) const
    {
        assert(GetLength() >= sizeof(aValue));
        aValue = Encoding::BigEndian::Get32(GetValue());
    }

    void Get16(uint16_t & aValue) const
    {
        assert(GetLength() >= sizeof(aValue));
        aValue = Encoding::BigEndian::Get16(GetValue());
    }

    void Set64(uint64_t aValue)
    {
        SetLength(sizeof(aValue));
        Encoding::BigEndian::Put64(GetValue(), aValue);
    }

    void Set32(uint32_t aValue)
    {
        SetLength(sizeof(aValue));
        Encoding::BigEndian::Put32(GetValue(), aValue);
    }

    void Set16(uint16_t aValue)
    {
        SetLength(sizeof(aValue));
        Encoding::BigEndian::Put16(GetValue(), aValue);
    }

    void SetValue(const void * aValue, uint8_t aLength)
    {
        SetLength(aLength);
        memcpy(GetValue(), aValue, aLength);
    }

    void SetValue(const ByteSpan & aValue) { SetValue(aValue.data(), static_cast<uint8_t>(aValue.size())); }

    const ThreadTLV * GetNext() const
    {
        static_assert(alignof(ThreadTLV) == 1, "Wrong alignment for ThreadTLV header");
        return reinterpret_cast<const ThreadTLV *>(static_cast<const uint8_t *>(GetValue()) + GetLength());
    }

    ThreadTLV * GetNext() { return reinterpret_cast<ThreadTLV *>(static_cast<uint8_t *>(GetValue()) + GetLength()); }

    static bool IsValid(ByteSpan aData)
    {
        const uint8_t * const end = aData.data() + aData.size();
        const uint8_t * curr      = aData.data();

        while (curr + sizeof(ThreadTLV) < end)
        {
            const ThreadTLV * tlv = reinterpret_cast<const ThreadTLV *>(curr);

            if (tlv->GetLength() == kLengthEscape)
            {
                break;
            }

            curr = reinterpret_cast<const uint8_t *>(tlv->GetNext());
        }

        return curr == end;
    }

private:
    uint8_t mType;
    uint8_t mLength;
};

bool OperationalDataset::IsValid(ByteSpan aData)
{
    return ThreadTLV::IsValid(aData);
}

CHIP_ERROR OperationalDataset::Init(ByteSpan aData)
{
    if (aData.size() > sizeof(mData))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (aData.size() > 0)
    {
        if (!ThreadTLV::IsValid(aData))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        memcpy(mData, aData.data(), aData.size());
    }

    mLength = static_cast<uint8_t>(aData.size());
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::GetActiveTimestamp(uint64_t & aActiveTimestamp) const
{
    const ThreadTLV * tlv = Locate(ThreadTLV::kActiveTimestamp);
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_TLV_TAG_NOT_FOUND);
    VerifyOrReturnError(tlv->GetLength() == sizeof(aActiveTimestamp), CHIP_ERROR_INVALID_TLV_ELEMENT);
    tlv->Get64(aActiveTimestamp);
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::SetActiveTimestamp(uint64_t aActiveTimestamp)
{
    ThreadTLV * tlv = MakeRoom(ThreadTLV::kActiveTimestamp, sizeof(*tlv) + sizeof(aActiveTimestamp));
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_NO_MEMORY);

    tlv->Set64(aActiveTimestamp);

    mLength = static_cast<uint8_t>(mLength + tlv->GetSize());

    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::GetChannel(uint16_t & aChannel) const
{
    const ThreadTLV * tlv = Locate(ThreadTLV::kChannel);
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_TLV_TAG_NOT_FOUND);
    VerifyOrReturnError(tlv->GetLength() == 3, CHIP_ERROR_INVALID_TLV_ELEMENT);
    // Note: The channel page (byte 0) is not returned
    const uint8_t * value = tlv->GetValue();
    aChannel              = static_cast<uint16_t>((value[1] << 8) | value[2]);
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::SetChannel(uint16_t aChannel)
{
    uint8_t value[] = { 0, static_cast<uint8_t>(aChannel >> 8), static_cast<uint8_t>(aChannel & 0xff) };
    ThreadTLV * tlv = MakeRoom(ThreadTLV::kChannel, sizeof(*tlv) + sizeof(value));
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_NO_MEMORY);

    tlv->SetValue(value, sizeof(value));

    mLength = static_cast<uint8_t>(mLength + tlv->GetSize());

    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::GetExtendedPanId(uint8_t (&aExtendedPanId)[kSizeExtendedPanId]) const
{
    ByteSpan extPanIdSpan;
    ReturnErrorOnFailure(GetExtendedPanIdAsByteSpan(extPanIdSpan));
    memcpy(aExtendedPanId, extPanIdSpan.data(), extPanIdSpan.size());
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::GetExtendedPanId(uint64_t & extendedPanId) const
{
    ByteSpan extPanIdSpan;
    ReturnErrorOnFailure(GetExtendedPanIdAsByteSpan(extPanIdSpan));
    VerifyOrDie(extPanIdSpan.size() == sizeof(extendedPanId));
    extendedPanId = Encoding::BigEndian::Get64(extPanIdSpan.data());
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::GetExtendedPanIdAsByteSpan(ByteSpan & span) const
{
    const ThreadTLV * tlv = Locate(ThreadTLV::kExtendedPanId);
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_TLV_TAG_NOT_FOUND);
    VerifyOrReturnError(tlv->GetLength() == kSizeExtendedPanId, CHIP_ERROR_INVALID_TLV_ELEMENT);
    span = tlv->GetValueAsSpan();
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::SetExtendedPanId(const uint8_t (&aExtendedPanId)[kSizeExtendedPanId])
{
    ThreadTLV * tlv = MakeRoom(ThreadTLV::kExtendedPanId, sizeof(*tlv) + sizeof(aExtendedPanId));
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_NO_MEMORY);

    tlv->SetValue(aExtendedPanId, sizeof(aExtendedPanId));

    assert(mLength + tlv->GetSize() <= sizeof(mData));

    mLength = static_cast<uint8_t>(mLength + tlv->GetSize());

    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::GetMasterKey(uint8_t (&aMasterKey)[kSizeMasterKey]) const
{
    const ThreadTLV * tlv = Locate(ThreadTLV::kMasterKey);
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_TLV_TAG_NOT_FOUND);
    VerifyOrReturnError(tlv->GetLength() == sizeof(aMasterKey), CHIP_ERROR_INVALID_TLV_ELEMENT);
    memcpy(aMasterKey, tlv->GetValue(), sizeof(aMasterKey));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::SetMasterKey(const uint8_t (&aMasterKey)[kSizeMasterKey])
{
    ThreadTLV * tlv = MakeRoom(ThreadTLV::kMasterKey, sizeof(*tlv) + sizeof(aMasterKey));
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_NO_MEMORY);

    tlv->SetValue(aMasterKey, sizeof(aMasterKey));

    assert(mLength + tlv->GetSize() <= sizeof(mData));

    mLength = static_cast<uint8_t>(mLength + tlv->GetSize());

    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::GetMeshLocalPrefix(uint8_t (&aMeshLocalPrefix)[kSizeMeshLocalPrefix]) const
{
    const ThreadTLV * tlv = Locate(ThreadTLV::kMeshLocalPrefix);
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_TLV_TAG_NOT_FOUND);
    VerifyOrReturnError(tlv->GetLength() == sizeof(aMeshLocalPrefix), CHIP_ERROR_INVALID_TLV_ELEMENT);
    memcpy(aMeshLocalPrefix, tlv->GetValue(), sizeof(aMeshLocalPrefix));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::SetMeshLocalPrefix(const uint8_t (&aMeshLocalPrefix)[kSizeMeshLocalPrefix])
{
    ThreadTLV * tlv = MakeRoom(ThreadTLV::kMeshLocalPrefix, sizeof(*tlv) + sizeof(aMeshLocalPrefix));
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_NO_MEMORY);

    tlv->SetValue(aMeshLocalPrefix, sizeof(aMeshLocalPrefix));

    mLength = static_cast<uint8_t>(mLength + tlv->GetSize());

    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::GetNetworkName(char (&aNetworkName)[kSizeNetworkName + 1]) const
{
    const ThreadTLV * tlv = Locate(ThreadTLV::kNetworkName);
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_TLV_TAG_NOT_FOUND);
    VerifyOrReturnError(tlv->GetLength() <= kSizeNetworkName, CHIP_ERROR_INVALID_TLV_ELEMENT);
    memcpy(aNetworkName, tlv->GetValue(), tlv->GetLength());
    aNetworkName[tlv->GetLength()] = '\0';
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::SetNetworkName(const char * aNetworkName)
{
    VerifyOrReturnError(aNetworkName != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    size_t len = strlen(aNetworkName);
    VerifyOrReturnError(0 < len && len <= kSizeNetworkName, CHIP_ERROR_INVALID_STRING_LENGTH);

    ThreadTLV * tlv = MakeRoom(ThreadTLV::kNetworkName, sizeof(*tlv) + len);
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_NO_MEMORY);

    tlv->SetValue(aNetworkName, static_cast<uint8_t>(len));

    mLength = static_cast<uint8_t>(mLength + tlv->GetSize());

    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::GetPanId(uint16_t & aPanId) const
{
    const ThreadTLV * tlv = Locate(ThreadTLV::kPanId);
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_TLV_TAG_NOT_FOUND);
    VerifyOrReturnError(tlv->GetLength() == sizeof(aPanId), CHIP_ERROR_INVALID_TLV_ELEMENT);
    tlv->Get16(aPanId);
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::SetPanId(uint16_t aPanId)
{
    ThreadTLV * tlv = MakeRoom(ThreadTLV::kPanId, sizeof(*tlv) + sizeof(aPanId));
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_NO_MEMORY);

    tlv->Set16(aPanId);

    mLength = static_cast<uint8_t>(mLength + tlv->GetSize());

    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::GetPSKc(uint8_t (&aPSKc)[kSizePSKc]) const
{
    const ThreadTLV * tlv = Locate(ThreadTLV::kPSKc);
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_TLV_TAG_NOT_FOUND);
    VerifyOrReturnError(tlv->GetLength() == sizeof(aPSKc), CHIP_ERROR_INVALID_TLV_ELEMENT);
    memcpy(aPSKc, tlv->GetValue(), sizeof(aPSKc));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::SetPSKc(const uint8_t (&aPSKc)[kSizePSKc])
{
    ThreadTLV * tlv = MakeRoom(ThreadTLV::kPSKc, sizeof(*tlv) + sizeof(aPSKc));
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_NO_MEMORY);

    tlv->SetValue(aPSKc, sizeof(aPSKc));

    mLength = static_cast<uint8_t>(mLength + tlv->GetSize());

    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::GetChannelMask(ByteSpan & aChannelMask) const
{
    const ThreadTLV * tlv = Locate(ThreadTLV::kChannelMask);
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_TLV_TAG_NOT_FOUND);
    VerifyOrReturnError(tlv->GetLength() > 0, CHIP_ERROR_INVALID_TLV_ELEMENT);
    aChannelMask = tlv->GetValueAsSpan();
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::SetChannelMask(ByteSpan aChannelMask)
{
    VerifyOrReturnError(0 < aChannelMask.size() && aChannelMask.size() < ThreadTLV::kMaxLength, CHIP_ERROR_INVALID_ARGUMENT);
    ThreadTLV * tlv = MakeRoom(ThreadTLV::kChannelMask, sizeof(*tlv) + aChannelMask.size());
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_NO_MEMORY);
    tlv->SetValue(aChannelMask);
    mLength = static_cast<uint8_t>(mLength + tlv->GetSize());
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::GetSecurityPolicy(uint32_t & aSecurityPolicy) const
{
    const ThreadTLV * tlv = Locate(ThreadTLV::kSecurityPolicy);
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_TLV_TAG_NOT_FOUND);
    VerifyOrReturnError(tlv->GetLength() == sizeof(aSecurityPolicy), CHIP_ERROR_INVALID_TLV_ELEMENT);
    tlv->Get32(aSecurityPolicy);
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::SetSecurityPolicy(uint32_t aSecurityPolicy)
{
    ThreadTLV * tlv = MakeRoom(ThreadTLV::kSecurityPolicy, sizeof(*tlv) + sizeof(aSecurityPolicy));
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_NO_MEMORY);
    tlv->Set32(aSecurityPolicy);
    mLength = static_cast<uint8_t>(mLength + tlv->GetSize());
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::GetDelayTimer(uint32_t & aDelayMillis) const
{
    const ThreadTLV * tlv = Locate(ThreadTLV::kDelayTimer);
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_TLV_TAG_NOT_FOUND);
    VerifyOrReturnError(tlv->GetLength() == sizeof(aDelayMillis), CHIP_ERROR_INVALID_TLV_ELEMENT);
    tlv->Get32(aDelayMillis);
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::SetDelayTimer(uint32_t aDelayMillis)
{
    ThreadTLV * tlv = MakeRoom(ThreadTLV::kDelayTimer, sizeof(*tlv) + sizeof(aDelayMillis));
    VerifyOrReturnError(tlv != nullptr, CHIP_ERROR_NO_MEMORY);
    tlv->Set32(aDelayMillis);
    mLength = static_cast<uint8_t>(mLength + tlv->GetSize());
    return CHIP_NO_ERROR;
}

void OperationalDataset::UnsetMasterKey()
{
    Remove(ThreadTLV::kMasterKey);
}

void OperationalDataset::UnsetPSKc()
{
    Remove(ThreadTLV::kPSKc);
}

bool OperationalDataset::IsCommissioned() const
{
    return Has(ThreadTLV::kPanId) && Has(ThreadTLV::kMasterKey) && Has(ThreadTLV::kExtendedPanId) && Has(ThreadTLV::kChannel);
}

const ThreadTLV * OperationalDataset::Locate(uint8_t aType) const
{
    const ThreadTLV * tlv = &Begin();
    const ThreadTLV * end = &End();

    while (tlv < end)
    {
        if (tlv->GetType() == aType)
            break;
        tlv = tlv->GetNext();
    }

    assert(tlv < reinterpret_cast<const ThreadTLV *>(&mData[sizeof(mData)]));

    return tlv != end ? tlv : nullptr;
}

void OperationalDataset::Remove(ThreadTLV & aThreadTLV)
{
    uint8_t offset = static_cast<uint8_t>(reinterpret_cast<uint8_t *>(&aThreadTLV) - mData);

    if (offset < mLength && mLength >= (offset + aThreadTLV.GetSize()))
    {
        mLength = static_cast<uint8_t>(mLength - aThreadTLV.GetSize());
        memmove(&aThreadTLV, aThreadTLV.GetNext(), mLength - offset);
    }
}

void OperationalDataset::Remove(uint8_t aType)
{
    ThreadTLV * tlv = Locate(aType);

    if (tlv != nullptr)
    {
        Remove(*tlv);
    }
}

ThreadTLV * OperationalDataset::MakeRoom(uint8_t aType, size_t aSize)
{
    ThreadTLV * tlv = Locate(aType);

    size_t freeSpace = sizeof(mData) - mLength;

    if (tlv != nullptr)
    {
        if (freeSpace + tlv->GetSize() < aSize)
        {
            return nullptr;
        }

        Remove(*tlv);
    }
    else if (freeSpace < aSize)
    {
        return nullptr;
    }

    End().SetType(aType);

    return &End();
}

} // namespace Thread
} // namespace chip
