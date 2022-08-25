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

#include <app/AttributeAccessInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>

#include <stdbool.h>
#include <stdint.h>

#include <functional>

static constexpr uint16_t ZCL_DESCRIPTOR_CLUSTER_REVISION              = 1;
static constexpr uint16_t ZCL_BRIDGED_DEVICE_BASIC_CLUSTER_REVISION    = 1;
static constexpr uint16_t ZCL_FIXED_LABEL_CLUSTER_REVISION             = 1;
static constexpr uint16_t ZCL_ON_OFF_CLUSTER_REVISION                  = 4;
static constexpr uint16_t ZCL_SWITCH_CLUSTER_REVISION                  = 1;
static constexpr uint16_t ZCL_LEVEL_CONTROL_CLUSTER_REVISION           = 1;
static constexpr uint16_t ZCL_TEMPERATURE_MEASUREMENT_CLUSTER_REVISION = 1;

// This is the interface to cluster implementations, providing access to manipulate attributes.
class ClusterImpl
{
public:
    virtual ~ClusterImpl() = default;

    virtual void SetEndpointId(chip::EndpointId id)                          = 0;
    virtual chip::ClusterId GetClusterId()                                   = 0;
    virtual chip::Span<const EmberAfAttributeMetadata> GetAllAttributes()    = 0;
    virtual bool Push(chip::AttributeId attr, chip::TLV::TLVReader & reader) = 0;
};

// This provides storage for octet strings
template<size_t Bytes, bool IsChar>
struct FixedOctetString
{
    CHIP_ERROR Decode(chip::TLV::TLVReader & reader)
    {
        chip::ByteSpan v;
        CHIP_ERROR err = chip::app::DataModel::Decode(reader, v);
        if (err == CHIP_NO_ERROR)
        {
            if (v.size() > Bytes)
            {
                return CHIP_ERROR_BUFFER_TOO_SMALL;
            }
            mLength = static_cast<uint16_t>(v.size());
            memcpy(mValue, v.data(), mLength);
        }
        return err;
    }

    CHIP_ERROR Encode(chip::TLV::TLVWriter & writer, chip::TLV::Tag tag) const
    {
        if (IsChar)
            return chip::app::DataModel::Encode(writer, tag, chip::CharSpan(static_cast<char *>(mValue), mLength));
        return chip::app::DataModel::Encode(writer, tag, chip::ByteSpan(mValue, mLength));
    }

    operator chip::ByteSpan() const { return chip::ByteSpan(mValue, mLength); }
    operator chip::CharSpan() const { return chip::CharSpan(static_cast<char *>(mValue), mLength); }

    void operator=(const FixedOctetString&) = default;

private:
    uint16_t mLength = 0;
    uint8_t mValue[Bytes];
};

// This allows for an array of up to MaxElements of a specified type.
template <size_t MaxElements, bool Nullable, chip::AttributeId id, EmberAfAttributeMask mask, EmberAfAttributeType AfType, size_t Bytes, typename Type, bool IsFabricScoped>
struct ArrayAttribute
{
    static constexpr chip::AttributeId kId    = id;
    static constexpr EmberAfClusterMask kMask = mask | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE);
    static constexpr EmberAfAttributeType kMatterType = AfType;
    // Bytes is the total length, including the length prefix
    static constexpr uint16_t kMaxSize = Bytes;
    static_assert(Bytes <= MaxElements * sizeof(Type) + 2, "Incorrect type / byte specification");
    static_assert(std::is_standard_layout<Type>::value, "Array element not standard layout!");

    static constexpr bool kIsFabricScoped = IsFabricScoped;

    void ListWriteBegin(const chip::app::ConcreteAttributePath & aPath) {}

    void ListWriteEnd(const chip::app::ConcreteAttributePath & aPath, bool aWriteWasSuccessful) {}

    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder)
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder)
    {
        if (aPath.mListIndex.HasValue())
        {
            uint16_t index = aPath.mListIndex.Value();
            if (index >= mLength || IsNull())
                return CHIP_ERROR_INVALID_ARGUMENT;

            return aEncoder.Encode(mArray[index]);
        }

        if (IsNull())
            return aEncoder.EncodeNull();
        if (mLength == 0)
            return aEncoder.EncodeEmptyList();
        return aEncoder.EncodeList([this](const auto & encoder) {
            CHIP_ERROR err = CHIP_NO_ERROR;
            for (uint16_t i = 0; i < mLength && err == CHIP_NO_ERROR; i++) {
                err = encoder.Encode(mArray[i]);
            }
            return err;
        });
    }

    bool IsNull() const { return mLength == 0xFFFF; }

private:
    uint16_t mLength = Nullable ? 0xFFFF : 0;
    Type mArray[MaxElements];
};

// This is used by code generation to provide specific attributes. Type should be a primitive that works with Encode()/Decode().
template <chip::AttributeId id, EmberAfAttributeMask mask, EmberAfAttributeType AfType, size_t Bytes, typename Type, bool IsFabricScoped>
struct Attribute
{
    static constexpr chip::AttributeId kId    = id;
    static constexpr EmberAfClusterMask kMask = mask | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE);
    static constexpr EmberAfAttributeType kMatterType = AfType;
    // Note that Bytes can be < sizeof(T) due to non-power-of-two byte lengths.
    static constexpr uint16_t kMaxSize = Bytes;
    static_assert(Bytes <= sizeof(Type), "Incorrect type / byte specification");

    static constexpr bool kIsFabricScoped = IsFabricScoped;

    Attribute(Type v = Type()) : mData(v) {}

    const Type& Peek() const
    {
        return mData;
    }
    void operator=(const Type& v)
    {
        mData = v;
    }

    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder)
    {
        return aDecoder.Decode(mData);
    }

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder)
    {
        return aEncoder.Encode(mData);
    }

    Type mData;
};

struct CommonCluster;
typedef std::function<CHIP_ERROR(CommonCluster *, const chip::app::ConcreteDataAttributePath &, chip::app::AttributeValueDecoder &)>
    PropagateWriteCB;

// This is the base type of all generated clusters, providing the backend access to
// implementation details.
struct CommonCluster : public ClusterImpl
{
    void SetEndpointId(chip::EndpointId id) override;
    chip::EndpointId GetEndpointId() const;

    void SetCallback(PropagateWriteCB * cb);

    bool active() const;

    virtual CHIP_ERROR WriteFromBridge(const chip::app::ConcreteDataAttributePath & aPath,
                                       chip::app::AttributeValueDecoder & aDecoder) = 0;

    CHIP_ERROR ForwardWriteToBridge(const chip::app::ConcreteDataAttributePath & aPath,
                                    chip::app::AttributeValueDecoder & aDecoder);

    template <typename T, typename U>
    void Push(T & attr, U v)
    {
        attr = v;
        OnUpdated(T::kId);
    }
    bool Push(chip::AttributeId attr, chip::TLV::TLVReader & reader);

    void OnUpdated(chip::AttributeId attr);
    template <typename T>
    void OnUpdated(const T & attr)
    {
        OnUpdated(T::kId);
    }

protected:
    chip::EndpointId mEndpoint   = 0xFFFF;
    PropagateWriteCB * mCallback = nullptr;
};

struct CommonAttributeAccessInterface : public chip::app::AttributeAccessInterface
{
    using chip::app::AttributeAccessInterface::AttributeAccessInterface;
    static CommonCluster * FindCluster(const chip::app::ConcreteClusterPath & path);
};

#include "bridge/BridgeClustersImpl.h"

namespace clusters {
struct BridgedDeviceBasicCluster : public CommonCluster
{
    static constexpr chip::ClusterId kClusterId = ZCL_BRIDGED_DEVICE_BASIC_CLUSTER_ID;

    BridgedDeviceBasicCluster();

    chip::ClusterId GetClusterId() override { return kClusterId; }
    CHIP_ERROR WriteFromBridge(const chip::app::ConcreteDataAttributePath & aPath,
                               chip::app::AttributeValueDecoder & aDecoder) override;

    template <typename T>
    void AddAllAttributes(T * list)
    {
        list->Add(mReachable);
    }

    chip::Span<const EmberAfAttributeMetadata> GetAllAttributes() override
    {
        static constexpr EmberAfAttributeMetadata kAllAttributes[] = {
            { ZCL_REACHABLE_ATTRIBUTE_ID, ZCL_BOOLEAN_ATTRIBUTE_TYPE, 1, 0, 0u },
        };
        return chip::Span<const EmberAfAttributeMetadata>(kAllAttributes);
    }

    Attribute<ZCL_REACHABLE_ATTRIBUTE_ID, 0, ZCL_BOOLEAN_ATTRIBUTE_TYPE, 1, uint8_t, false> mReachable;
};

} // namespace clusters
