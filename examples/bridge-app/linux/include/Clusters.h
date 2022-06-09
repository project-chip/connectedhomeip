/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *    All rights reserved.
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

#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>

#include <stdbool.h>
#include <stdint.h>

#include <functional>

static constexpr uint16_t ZCL_DESCRIPTOR_CLUSTER_REVISION           = 1;
static constexpr uint16_t ZCL_BRIDGED_DEVICE_BASIC_CLUSTER_REVISION = 1;
static constexpr uint16_t ZCL_FIXED_LABEL_CLUSTER_REVISION          = 1;
static constexpr uint16_t ZCL_ON_OFF_CLUSTER_REVISION               = 4;
static constexpr uint16_t ZCL_SWITCH_CLUSTER_REVISION               = 1;
static constexpr uint16_t ZCL_LEVEL_CONTROL_CLUSTER_REVISION        = 1;

class ClusterImpl
{
public:
    virtual ~ClusterImpl() = default;

    virtual void SetEndpointId(chip::EndpointId id)                                                                          = 0;
    virtual chip::ClusterId GetClusterId()                                                                                   = 0;
    virtual EmberAfStatus Read(const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer, uint16_t maxReadLength) = 0;
    virtual EmberAfStatus Write(const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer)                        = 0;
    virtual void WriteFromBridge(chip::AttributeId attributeId, const uint8_t * buffer)                                      = 0;
    virtual chip::Span<const EmberAfAttributeMetadata> GetAllAttributes()                                                    = 0;
};

template <typename T, size_t Bytes, EmberAfAttributeType AfType>
struct PrimitiveType
{
    static constexpr EmberAfAttributeType kMatterType = AfType;
    // Note that Bytes can be < sizeof(T) due to non-power-of-two byte lengths.
    static constexpr uint16_t kMaxSize = Bytes;
    static_assert(Bytes <= sizeof(T), "Incorrect type / byte specification");
    typedef T ValueType;

    PrimitiveType() = default;
    PrimitiveType(T initial) : mValue(initial) {}

    EmberAfStatus Read(uint8_t * buffer, uint16_t sz) const
    {
        if (sz != kMaxSize)
            return EMBER_ZCL_STATUS_FAILURE;
        memcpy(buffer, &mValue, kMaxSize);
        return EMBER_ZCL_STATUS_SUCCESS;
    }

    bool Equal(const uint8_t * buffer) const { return !memcmp(buffer, &mValue, kMaxSize); }

    void Write(const uint8_t * buffer) { memcpy(&mValue, buffer, kMaxSize); }

    bool TryWrite(ValueType newV)
    {
        if (mValue == newV)
            return false;
        mValue = newV;
        return true;
    }
    bool TryWrite(const uint8_t * buffer)
    {
        if (Equal(buffer))
            return false;
        Write(buffer);
        return true;
    }

    const ValueType & Peek() const { return mValue; }

    uint8_t * GetBytes() { return reinterpret_cast<uint8_t *>(&mValue); }

private:
    ValueType mValue;
};

template <typename T>
using StructType = PrimitiveType<T, sizeof(T), ZCL_STRUCT_ATTRIBUTE_TYPE>;

template <size_t Bytes, EmberAfAttributeType AfType>
struct OctetString
{
    static constexpr EmberAfAttributeType kMatterType = AfType;
    static constexpr bool kIsLong =
        (AfType == ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE || AfType == ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE);
    static constexpr uint16_t kMaxSize = Bytes;
    typedef const uint8_t * ValueType;

    OctetString() = default;

    EmberAfStatus Read(uint8_t * buffer, uint16_t sz) const
    {
        // There should be at least enough space to hold the size
        if (kIsLong && sz < 2)
            return EMBER_ZCL_STATUS_FAILURE;
        memcpy(buffer, mValue, sz < kMaxSize ? sz : kMaxSize);
        return EMBER_ZCL_STATUS_SUCCESS;
    }

    bool Equal(const uint8_t * buffer)
    {
        uint16_t newsz = GetSize(buffer);
        if (newsz != GetSize(mValue))
            return false;
        return memcmp(buffer, mValue, newsz);
    }

    void Write(const uint8_t * buffer) { memcpy(mValue, buffer, GetSize(buffer)); }

    bool TryWrite(const uint8_t * newV)
    {
        if (Equal(newV))
            return false;
        Write(newV);
        return true;
    }

    ValueType Peek() const { return mValue; }

    uint8_t * GetBytes() { return mValue; }

private:
    uint16_t GetSize(const uint8_t * buf)
    {
        if (!kIsLong)
            return *buf + 1;
        uint16_t sz;
        memcpy(&sz, buf, 2);
        return sz + 2;
    }
    uint8_t mValue[Bytes];
};

template <size_t MaxElements, typename T>
struct ArrayType
{
    static constexpr EmberAfAttributeType kMatterType = ZCL_ARRAY_ATTRIBUTE_TYPE;
    struct Data
    {
        uint16_t length;
        T array[MaxElements];
    };
    // We need this to be standard layout to get a reliable byte view.
    static_assert(std::is_standard_layout<Data>::value, "Array of elements not standard layout!");

private:
    Data mValue;
};

struct CommonCluster;

typedef std::function<EmberAfStatus(CommonCluster *, chip::EndpointId, chip::ClusterId, chip::AttributeId, const uint8_t *)>
    PropagateWriteCB;

struct CommonCluster : public ClusterImpl
{
    void SetEndpointId(chip::EndpointId id) override { mEndpoint = id; }
    chip::EndpointId GetEndpointId() const { return mEndpoint; }

    EmberAfStatus WriteCallback(chip::AttributeId id, uint8_t * buffer);

    void SetCallback(PropagateWriteCB * cb) { mCallback = cb; }

    bool active() const { return mEndpoint < 0xFFFF; }

protected:
    bool mActive                 = false;
    chip::EndpointId mEndpoint   = 0xFFFF;
    PropagateWriteCB * mCallback = nullptr;
};

template <chip::AttributeId id, EmberAfAttributeMask mask, typename Type>
struct Attribute
{
    static constexpr chip::AttributeId kId            = id;
    static constexpr EmberAfClusterMask kMask         = mask | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE);
    static constexpr uint16_t kMaxSize                = Type::kMaxSize;
    static constexpr EmberAfAttributeType kMatterType = Type::kMatterType;

    Attribute() : mValue() {}
    template <typename U>
    Attribute(U v) : mValue(v)
    {}

    bool IsAttribute(const EmberAfAttributeMetadata * attributeMetadata) const { return attributeMetadata->attributeId == kId; }
    bool IsAttribute(chip::AttributeId attrId) const { return attrId == kId; }
    EmberAfStatus Read(const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer, uint16_t maxReadLength) const
    {
        // Length of 0 means read everything
        return mValue.Read(buffer, maxReadLength ? maxReadLength : kMaxSize);
    }
    EmberAfStatus WriteFromMatter(const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer, CommonCluster * cluster)
    {
        if (!(kMask & ATTRIBUTE_MASK_WRITABLE))
            return EMBER_ZCL_STATUS_READ_ONLY;

        if (mValue.Equal(buffer))
            return EMBER_ZCL_STATUS_SUCCESS;

        // Don't directly write the value. The theory of operation of the bridge is that
        // some external device is being presented, so we shouldn't change matter-visible
        // attributes until the backend has had a chance to intercept it first.
        return cluster->WriteCallback(attributeMetadata->attributeId, buffer);
    }
    template <typename T>
    void WriteFromBridge(const T & value, CommonCluster * cluster)
    {
        if (mValue.TryWrite(value) && cluster->active())
        {
            OnChanged(cluster);
        }
    }

    auto Peek() const { return mValue.Peek(); }

protected:
    void OnChanged(CommonCluster * cluster)
    {
        MatterReportingAttributeChangeCallback(cluster->GetEndpointId(), cluster->GetClusterId(), kId, kMatterType,
                                               mValue.GetBytes());
    }

    Type mValue;
};

#include "cpp/BridgeClustersImpl.h"

namespace clusters {
struct BridgedDeviceBasicCluster : public CommonCluster
{
    static constexpr chip::ClusterId kClusterId = ZCL_BRIDGED_DEVICE_BASIC_CLUSTER_ID;

    BridgedDeviceBasicCluster();

    chip::ClusterId GetClusterId() override { return kClusterId; }
    EmberAfStatus Read(const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer, uint16_t maxReadLength) override;
    EmberAfStatus Write(const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer) override;
    void WriteFromBridge(chip::AttributeId attributeId, const uint8_t * buffer) override;

    template <typename T>
    void AddAllAttributes(T * list)
    {
        list->Add(mReachable);
    }

    static constexpr EmberAfAttributeMetadata kAllAttributes[] = {
        { ZCL_REACHABLE_ATTRIBUTE_ID, ZCL_BOOLEAN_ATTRIBUTE_TYPE, 1, 0, 0u },
    };
    chip::Span<const EmberAfAttributeMetadata> GetAllAttributes() override
    {
        return chip::Span<const EmberAfAttributeMetadata>(kAllAttributes);
    }

    Attribute<ZCL_REACHABLE_ATTRIBUTE_ID, 0, PrimitiveType<uint8_t, 1, ZCL_BOOLEAN_ATTRIBUTE_TYPE>> mReachable;
};

} // namespace clusters
