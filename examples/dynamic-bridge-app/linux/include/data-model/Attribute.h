#pragma once

#include <app/ConcreteAttributePath.h>

#include "DataModel.h"

// This is the interface to the attribute implementation that permits retrieval of metadata about
// the attribute as well as set/get the underlying value.
class AttributeInterface
{
public:
    virtual ~AttributeInterface() = default;

    // Get the ID of this attribute. Shortcut for GetMetadata().attributeId
    virtual chip::AttributeId GetId() = 0;
    // Get the metadata of this attribute, suitable for advertising.
    virtual const EmberAfAttributeMetadata & GetMetadata() = 0;

    virtual chip::CharSpan GetName() = 0;

    // List attribes should override these to get list write notifications.
    virtual void ListWriteBegin(const chip::app::ConcreteAttributePath & aPath) {}
    virtual void ListWriteEnd(const chip::app::ConcreteAttributePath & aPath, bool aWriteWasSuccessful) {}

    // Write a single value to the attribute.
    virtual CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) = 0;

    // Read the contents of the attribute.
    virtual CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) = 0;

    virtual CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::TLV::TLVWriter & writer) = 0;
};

// This is the base type for implementing an attribute
template <typename Type>
struct Attribute : public AttributeInterface
{
    Attribute(chip::CharSpan name, chip::AttributeId id, EmberAfAttributeMask mask, EmberAfAttributeType type, size_t size,
              Type value = Type()) :
        mMetadata(EmberAfAttributeMetadata{ ZAP_EMPTY_DEFAULT(), id, (uint16_t) size, type, mask }),
        mData(value), mName(name)
    {}

    chip::AttributeId GetId() override { return mMetadata.attributeId; }
    const EmberAfAttributeMetadata & GetMetadata() override { return mMetadata; }
    chip::CharSpan GetName() override { return mName; }

    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override
    {
        return chip::app::DataModel::Decode(aPath, aDecoder, mData);
    }
    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override
    {
        return chip::app::DataModel::Encode(aPath, aEncoder, mData);
    }

    template <typename T = Type, std::enable_if_t<chip::app::DataModel::IsList<std::decay_t<T>>::value, bool> = true>
    CHIP_ERROR ReadValue(const chip::app::ConcreteReadAttributePath & aPath, chip::TLV::TLVWriter & writer, Type & value)
    {
        return chip::app::DataModel::Encode(aPath, writer, chip::TLV::AnonymousTag(), value);
    }

    template <typename T = Type, std::enable_if_t<!chip::app::DataModel::IsList<std::decay_t<T>>::value, bool> = true>
    CHIP_ERROR ReadValue(const chip::app::ConcreteReadAttributePath & aPath, chip::TLV::TLVWriter & writer, Type & value)
    {
        return chip::app::DataModel::Encode(writer, chip::TLV::AnonymousTag(), value);
    }

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::TLV::TLVWriter & writer) override
    {
        return ReadValue(aPath, writer, mData);
    }

    void operator=(const Type & value) { mData = value; }
    const Type & Peek() const { return mData; }

protected:
    EmberAfAttributeMetadata mMetadata;
    Type mData;
    chip::CharSpan mName;
};

// This specialization handles list writes and reverts if the write fails.
template <typename Type>
struct ListAttribute : public Attribute<Type>
{
    using Attribute<Type>::Attribute;

    void ListWriteBegin(const chip::app::ConcreteAttributePath & aPath) override { mBackup = this->mData; }
    void ListWriteEnd(const chip::app::ConcreteAttributePath & aPath, bool aWriteWasSuccessful) override
    {
        if (aWriteWasSuccessful)
        {
            mBackup = Type();
        }
        else
        {
            this->mData = std::move(mBackup);
        }
    }

    Type mBackup;
};
