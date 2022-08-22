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

// This provides storage for a primitive binary type, eg uintN_t, float, double
template <typename T, size_t Bytes, EmberAfAttributeType AfType, bool IsFabricScoped = false>
struct PrimitiveType
{
    static constexpr EmberAfAttributeType kMatterType = AfType;
    // Note that Bytes can be < sizeof(T) due to non-power-of-two byte lengths.
    static constexpr uint16_t kMaxSize = Bytes;
    static_assert(Bytes <= sizeof(T), "Incorrect type / byte specification");

    static constexpr bool kIsFabricScoped = IsFabricScoped;

    PrimitiveType() = default;
    PrimitiveType(T initial) : mValue(initial) {}

    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder)
    {
        return aDecoder.Decode(*this);
    }
    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder)
    {
        return aEncoder.Encode(*this);
    }

    CHIP_ERROR Decode(chip::TLV::TLVReader & reader) { return reader.Get(mValue); }

    CHIP_ERROR Encode(chip::TLV::TLVWriter & writer, chip::TLV::Tag tag) const { return writer.Put(tag, mValue); }

    void operator=(T v) { mValue = v; }
    operator T() const { return mValue; }

    T Peek() const { return mValue; }

private:
    T mValue;
    static_assert(std::is_standard_layout<T>::value, "PrimitiveType not standard layout!");
};

// This provides access to a struct. Each struct generates its own access logic.
template <typename T, bool IsFabricScoped = false>
struct StructType : public T
{
    static constexpr EmberAfAttributeType kMatterType = ZCL_STRUCT_ATTRIBUTE_TYPE;
    static constexpr uint16_t kMaxSize                = sizeof(T);
    static_assert(std::is_standard_layout<T>::value, "StructType not standard layout!");
    static constexpr bool kIsFabricScoped = IsFabricScoped;

    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder)
    {
        return aDecoder.Decode(*this);
    }
    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder)
    {
        return aEncoder.Encode(*this);
    }
};

// This provides storage for short and long octet and char strings.
template <size_t Bytes, EmberAfAttributeType AfType>
struct OctetString
{
    static constexpr EmberAfAttributeType kMatterType = AfType;
    static constexpr bool kIsChar = (AfType == ZCL_CHAR_STRING_ATTRIBUTE_TYPE || AfType == ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE);
    static constexpr uint16_t kMaxSize = Bytes;

    OctetString() = default;

    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder)
    {
        chip::app::DataModel::Nullable<chip::ByteSpan> v;
        CHIP_ERROR err = aDecoder.Decode(v);
        if (err == CHIP_NO_ERROR)
        {
            if (v.IsNull() && !IsNull())
            {
                mLength = 0xFFFF;
            }
            else if (v.Value().size() > kMaxSize)
            {
                err = CHIP_ERROR_BUFFER_TOO_SMALL;
            }
            else
            {
                mLength = static_cast<uint16_t>(v.Value().size());
                memcpy(mValue, v.Value().data(), mLength);
            }
        }
        return err;
    }
    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder)
    {
        if (IsNull())
            return aEncoder.EncodeNull();
        if (kIsChar)
            return aEncoder.Encode(chip::CharSpan(static_cast<char *>(mValue), mLength));
        return aEncoder.Encode(chip::ByteSpan(mValue, mLength));
    }

    chip::ByteSpan Peek() const { return IsNull() ? chip::ByteSpan() : chip::ByteSpan(mValue, mLength); }

    bool IsNull() const { return mLength == 0xFFFF; }

private:
    uint16_t mLength = 0xFFFF;
    uint8_t mValue[kMaxSize];
};

// This allows for an array of up to MaxElements of a specified type.
template <size_t MaxElements, typename T>
struct ArrayType
{
    static constexpr EmberAfAttributeType kMatterType = ZCL_ARRAY_ATTRIBUTE_TYPE;
    static constexpr uint16_t kMaxSize                = sizeof(T) * MaxElements;
    static_assert(std::is_standard_layout<T>::value, "Array element not standard layout!");

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
            for (uint16_t i = 0; i < mLength && err == CHIP_NO_ERROR; i++)
                err = encoder.Encode(mArray[i]);
            return err;
        });
    }

    bool IsNull() const { return mLength == 0xFFFF; }

private:
    uint16_t mLength = 0xFFFF;
    T mArray[MaxElements];
};

// This is used by code generation to provide specific attributes. Type should be one of
// PrimitiveType, OctetString, ArrayType, etc to provide storage and type-specific
// functionality.
template <chip::AttributeId id, EmberAfAttributeMask mask, typename Type>
struct Attribute : public Type
{
    static constexpr chip::AttributeId kId    = id;
    static constexpr EmberAfClusterMask kMask = mask | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE);

    using Type::Type;
    using Type::operator=;
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

    Attribute<ZCL_REACHABLE_ATTRIBUTE_ID, 0, PrimitiveType<uint8_t, 1, ZCL_BOOLEAN_ATTRIBUTE_TYPE>> mReachable;
};

} // namespace clusters
