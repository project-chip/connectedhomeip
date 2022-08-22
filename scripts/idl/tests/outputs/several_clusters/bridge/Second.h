#pragma once

#include "BridgeGlobalStructs.h"

namespace clusters {
struct SecondCluster : public CommonCluster
{


  static constexpr chip::ClusterId kClusterId = 2;

  chip::ClusterId GetClusterId() override { return kClusterId; }

  CHIP_ERROR WriteFromBridge(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override
  {
    switch(aPath.mAttributeId)
    {
    case 123:
      return mSomeBytes.Write(aPath, aDecoder);
    default:
      return CHIP_ERROR_NOT_IMPLEMENTED;
    }
  }

  template<typename T>
  void AddAllAttributes(T *list)
  {
    list->Add(mSomeBytes);
  }

  chip::Span<const EmberAfAttributeMetadata> GetAllAttributes() override
  {
    static constexpr const EmberAfAttributeMetadata kAllAttributes[] = {
      { 123, ZCL_OCTET_STRING_ATTRIBUTE_TYPE, 32, 0 | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() },
    };
    return chip::Span<const EmberAfAttributeMetadata>(kAllAttributes);
  }


  Attribute<123, 0, OctetString<32, ZCL_OCTET_STRING_ATTRIBUTE_TYPE>> mSomeBytes;
};

struct SecondAccess : public CommonAttributeAccessInterface
{
  SecondAccess() : CommonAttributeAccessInterface(chip::Optional<chip::EndpointId>(), SecondCluster::kClusterId) {}

  SecondCluster* GetCluster(const chip::app::ConcreteClusterPath & aPath)
  {
    CommonCluster * cluster = FindCluster(aPath);
    return cluster ? static_cast<SecondCluster*>(cluster) : nullptr;
  }

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override
  {
    auto * c = GetCluster(aPath);
    if (!c)
      return CHIP_ERROR_NOT_IMPLEMENTED;

    switch(aPath.mAttributeId) {
    case 123:
      return c->mSomeBytes.Read(aPath, aEncoder);
    default:
      return CHIP_ERROR_NOT_IMPLEMENTED;
    }
  }

  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override
  {
    auto * c = GetCluster(aPath);
    if (!c)
      return CHIP_ERROR_NOT_IMPLEMENTED;
    return c->ForwardWriteToBridge(aPath, aDecoder);
  }

  void OnListWriteBegin(const chip::app::ConcreteAttributePath & aPath) override
  {
    auto * c = GetCluster(aPath);
    if (!c)
      return;

    switch(aPath.mAttributeId) {
    }
  }

  void OnListWriteEnd(const chip::app::ConcreteAttributePath & aPath, bool aWriteWasSuccessful) override
  {
    auto * c = GetCluster(aPath);
    if (!c)
      return;

    switch(aPath.mAttributeId) {
    }
  }
};

}