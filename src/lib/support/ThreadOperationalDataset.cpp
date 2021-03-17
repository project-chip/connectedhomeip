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

#include <assert.h>
#include <string.h>

#include "ThreadOperationalDataset.h"

namespace chip {
namespace Thread {

/**
 * This class implements Thread Tlv functionality.
 *
 */
class Tlv
{
    static constexpr uint8_t kLengthEscape = 0xff; ///< This length value indicates the actual length is of two-bytes length.

public:
    enum : uint8_t
    {
        kChannel         = 0,
        kPanId           = 1,
        kExtendedPanId   = 2,
        kNetworkName     = 3,
        kPSKc            = 4,
        kMasterKey       = 5,
        kMeshLocalPrefix = 7,
    };

    uint8_t GetSize(void) const { return static_cast<uint8_t>(sizeof(mType) + sizeof(mLength) + GetLength()); }

    /**
     * This method returns the Tlv type.
     *
     * @returns The Tlv type.
     *
     */
    uint8_t GetType(void) const { return mType; }

    /**
     * This method sets the Tlv type.
     *
     */
    void SetType(uint8_t aType) { mType = aType; }

    /**
     * This method returns the Tlv length.
     *
     * @returns The Tlv length.
     *
     */
    uint8_t GetLength(void) const { return mLength; }

    /**
     * This method sets the length.
     */
    void SetLength(uint8_t aLength) { mLength = aLength; }

    /**
     * This method returns a pointer to the value.
     *
     * @returns The Tlv value.
     *
     */
    const void * GetValue(void) const
    {
        return reinterpret_cast<const uint8_t *>(this) + sizeof(mType) +
            (mLength != kLengthEscape ? sizeof(mLength) : (sizeof(uint16_t) + sizeof(mLength)));
    }

    void * GetValue(void) { return const_cast<void *>(const_cast<const Tlv *>(this)->GetValue()); }

    /**
     * This method returns the value as a uint16_t.
     *
     * @returns The uint16_t value.
     *
     */
    uint16_t GetValueUInt16(void) const
    {
        const uint8_t * p = static_cast<const uint8_t *>(GetValue());

        return static_cast<uint16_t>(p[0] << 8 | p[1]);
    }

    /**
     * This method returns the value as a uint8_t.
     *
     * @returns The uint8_t value.
     *
     */
    uint8_t GetValueUInt8(void) const { return *static_cast<const uint8_t *>(GetValue()); }

    /**
     * This method sets uint16_t as the value.
     *
     * @param[in]    aValue         uint16_t value
     *
     */
    void SetValue(uint16_t aValue)
    {
        uint8_t * value;

        SetLength(sizeof(aValue));
        value    = static_cast<uint8_t *>(GetValue());
        value[0] = static_cast<uint8_t>(aValue >> 8);
        value[1] = static_cast<uint8_t>(aValue & 0xff);
    }

    /**
     * This method sets uint8_t as the value.
     *
     * @param[in]    aValue         uint8_t value
     *
     */
    void SetValue(uint8_t aValue)
    {
        SetLength(sizeof(aValue));
        *static_cast<uint8_t *>(GetValue()) = aValue;
    }

    /**
     * This method sets int8_t as the value.
     *
     * @param[in]    aValue         int8_t value
     *
     */
    void SetValue(int8_t aValue)
    {
        SetLength(sizeof(aValue));
        *static_cast<int8_t *>(GetValue()) = aValue;
    }

    /**
     * This method copies the value.
     */
    void SetValue(const void * aValue, uint8_t aLength)
    {
        SetLength(aLength);
        memcpy(GetValue(), aValue, aLength);
    }

    /**
     * This method returns the pointer to the next Tlv.
     *
     * @returns A pointer to the next Tlv.
     *
     */
    const Tlv * GetNext(void) const
    {
        return reinterpret_cast<const Tlv *>(static_cast<const uint8_t *>(GetValue()) + GetLength());
    }

    /**
     * This method returns the pointer to the next Tlv.
     *
     * @returns A pointer to the next Tlv.
     *
     */
    Tlv * GetNext(void) { return reinterpret_cast<Tlv *>(static_cast<uint8_t *>(GetValue()) + GetLength()); }

    static bool IsValid(const Tlv * begin, const Tlv * end)
    {
        while (reinterpret_cast<const uint8_t *>(begin) + sizeof(Tlv) < reinterpret_cast<const uint8_t *>(end))
        {
            begin = begin->GetNext();
        }

        return begin == end;
    }

private:
    uint8_t mType;
    uint8_t mLength;
};

CHIP_ERROR OperationalDataset::Init(const uint8_t * aData, uint8_t aLength)
{
    if (aLength > sizeof(mData))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (aLength > 0)
    {
        if (!Tlv::IsValid(reinterpret_cast<const Tlv *>(aData), reinterpret_cast<const Tlv *>(&aData[aLength])))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        memcpy(mData, aData, aLength);
    }

    mLength = aLength;
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::GetChannel(uint16_t & aChannel) const
{
    const Tlv * tlv = Locate(Tlv::kChannel);

    if (tlv != nullptr)
    {
        const uint8_t * value = reinterpret_cast<const uint8_t *>(tlv->GetValue());
        aChannel              = static_cast<uint16_t>((value[1] << 8) | value[2]);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_TLV_TAG_NOT_FOUND;
}

CHIP_ERROR OperationalDataset::SetChannel(uint16_t aChannel)
{
    uint8_t value[] = { 0, static_cast<uint8_t>(aChannel >> 8), static_cast<uint8_t>(aChannel & 0xff) };
    Tlv * tlv       = MakeRoom(Tlv::kChannel, sizeof(*tlv) + sizeof(value));

    if (tlv == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    tlv->SetValue(value, sizeof(value));

    mLength = static_cast<uint8_t>(mLength + tlv->GetSize());

    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::GetExtendedPanId(uint8_t (&aExtendedPanId)[kSizeExtendedPanId]) const
{
    const Tlv * tlv = Locate(Tlv::kExtendedPanId);

    if (tlv != nullptr)
    {
        memcpy(aExtendedPanId, tlv->GetValue(), sizeof(aExtendedPanId));
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_TLV_TAG_NOT_FOUND;
}

CHIP_ERROR OperationalDataset::SetExtendedPanId(const uint8_t (&aExtendedPanId)[kSizeExtendedPanId])
{
    Tlv * tlv = MakeRoom(Tlv::kExtendedPanId, sizeof(*tlv) + sizeof(aExtendedPanId));

    if (tlv == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    tlv->SetValue(aExtendedPanId, sizeof(aExtendedPanId));

    assert(mLength + tlv->GetSize() < sizeof(mData));

    mLength = static_cast<uint8_t>(mLength + tlv->GetSize());

    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::GetMasterKey(uint8_t (&aMasterKey)[kSizeMasterKey]) const
{
    const Tlv * tlv = Locate(Tlv::kMasterKey);

    if (tlv != nullptr)
    {
        memcpy(aMasterKey, tlv->GetValue(), sizeof(aMasterKey));
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_TLV_TAG_NOT_FOUND;
}

CHIP_ERROR OperationalDataset::SetMasterKey(const uint8_t (&aMasterKey)[kSizeMasterKey])
{
    Tlv * tlv = MakeRoom(Tlv::kMasterKey, sizeof(*tlv) + sizeof(aMasterKey));

    if (tlv == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    tlv->SetValue(aMasterKey, sizeof(aMasterKey));

    assert(mLength + tlv->GetSize() < sizeof(mData));

    mLength = static_cast<uint8_t>(mLength + tlv->GetSize());

    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::GetMeshLocalPrefix(uint8_t (&aMeshLocalPrefix)[kSizeMeshLocalPrefix]) const
{
    const Tlv * tlv = Locate(Tlv::kMeshLocalPrefix);

    if (tlv != nullptr)
    {
        memcpy(aMeshLocalPrefix, tlv->GetValue(), sizeof(aMeshLocalPrefix));
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_TLV_TAG_NOT_FOUND;
}

CHIP_ERROR OperationalDataset::SetMeshLocalPrefix(const uint8_t (&aMeshLocalPrefix)[kSizeMeshLocalPrefix])
{
    Tlv * tlv = MakeRoom(Tlv::kMeshLocalPrefix, sizeof(*tlv) + sizeof(aMeshLocalPrefix));

    if (tlv == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    tlv->SetValue(aMeshLocalPrefix, sizeof(aMeshLocalPrefix));

    mLength = static_cast<uint8_t>(mLength + tlv->GetSize());

    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::GetNetworkName(char (&aNetworkName)[kSizeNetworkName + 1]) const
{
    const Tlv * tlv = Locate(Tlv::kNetworkName);

    if (tlv != nullptr)
    {
        memcpy(aNetworkName, tlv->GetValue(), tlv->GetLength());
        aNetworkName[tlv->GetLength()] = '\0';
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_TLV_TAG_NOT_FOUND;
}

CHIP_ERROR OperationalDataset::SetNetworkName(const char * aNetworkName)
{
    size_t len = strlen(aNetworkName);

    if (len > kSizeNetworkName || len == 0)
    {
        return CHIP_ERROR_INVALID_STRING_LENGTH;
    }

    Tlv * tlv = MakeRoom(Tlv::kNetworkName, sizeof(*tlv) + len);

    if (tlv == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    tlv->SetValue(aNetworkName, static_cast<uint8_t>(len));

    mLength = static_cast<uint8_t>(mLength + tlv->GetSize());

    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::GetPanId(uint16_t & aPanId) const
{
    const Tlv * tlv = Locate(Tlv::kPanId);

    if (tlv != nullptr)
    {
        aPanId = tlv->GetValueUInt16();
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_TLV_TAG_NOT_FOUND;
}

CHIP_ERROR OperationalDataset::SetPanId(uint16_t aPanId)
{
    Tlv * tlv = MakeRoom(Tlv::kPanId, sizeof(*tlv) + sizeof(aPanId));

    if (tlv == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    tlv->SetValue(aPanId);

    mLength = static_cast<uint8_t>(mLength + tlv->GetSize());

    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDataset::GetPSKc(uint8_t (&aPSKc)[kSizePSKc]) const
{
    const Tlv * tlv = Locate(Tlv::kPSKc);

    if (tlv != nullptr)
    {
        memcpy(aPSKc, tlv->GetValue(), sizeof(aPSKc));
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_TLV_TAG_NOT_FOUND;
}

CHIP_ERROR OperationalDataset::SetPSKc(const uint8_t (&aPSKc)[kSizePSKc])
{
    Tlv * tlv = MakeRoom(Tlv::kPSKc, sizeof(*tlv) + sizeof(aPSKc));

    if (tlv == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    tlv->SetValue(aPSKc, sizeof(aPSKc));

    mLength = static_cast<uint8_t>(mLength + tlv->GetSize());

    return CHIP_NO_ERROR;
}

void OperationalDataset::UnsetMasterKey(void)
{
    Remove(Tlv::kMasterKey);
}

void OperationalDataset::UnsetPSKc(void)
{
    Remove(Tlv::kPSKc);
}

bool OperationalDataset::IsCommissioned(void) const
{
    return Has(Tlv::kNetworkName) && Has(Tlv::kPanId) && Has(Tlv::kMasterKey) && Has(Tlv::kExtendedPanId) && Has(Tlv::kChannel);
}

bool OperationalDataset::IsValid(void) const
{
    const Tlv * tlv = reinterpret_cast<const Tlv *>(mData);
    const Tlv * end = reinterpret_cast<const Tlv *>(&mData[mLength]);

    return Tlv::IsValid(tlv, end);
}

const Tlv * OperationalDataset::Locate(uint8_t aType) const
{
    const Tlv * tlv = reinterpret_cast<const Tlv *>(mData);
    const Tlv * end = &End();

    while (tlv < end)
    {
        if (tlv->GetType() == aType)
            break;
        else
            tlv = tlv->GetNext();
    }

    assert(tlv < reinterpret_cast<const Tlv *>(&mData[sizeof(mData)]));

    return tlv != end ? tlv : nullptr;
}

Tlv * OperationalDataset::Locate(uint8_t aType)
{
    return const_cast<Tlv *>(const_cast<const OperationalDataset *>(this)->Locate(aType));
}

void OperationalDataset::Remove(Tlv & aTlv)
{
    uint8_t offset = static_cast<uint8_t>(reinterpret_cast<uint8_t *>(&aTlv) - mData);

    if (offset < mLength && mLength >= (offset + aTlv.GetSize()))
    {
        mLength = static_cast<uint8_t>(mLength - aTlv.GetSize());
        memmove(&aTlv, aTlv.GetNext(), mLength - offset);
    }
}

void OperationalDataset::Remove(uint8_t aType)
{
    Tlv * tlv = Locate(aType);

    if (tlv != nullptr)
    {
        Remove(*tlv);
    }
}

Tlv * OperationalDataset::MakeRoom(uint8_t aType, size_t aSize)
{
    Tlv * tlv = Locate(aType);

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
