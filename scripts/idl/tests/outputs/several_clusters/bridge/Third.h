#pragma once

#include "BridgeGlobalStructs.h"

namespace clusters {
struct ThirdCluster : public CommonCluster
{


  static constexpr chip::ClusterId kClusterId = 3;

  chip::ClusterId GetClusterId() override { return kClusterId; }

  CHIP_ERROR WriteFromBridge(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override
  {
    switch(aPath.mAttributeId)
    {
    case 10:
      return mSomeEnum.Write(aPath, aDecoder);
    default:
      return CHIP_ERROR_NOT_IMPLEMENTED;
    }
  }

  template<typename T>
  void AddAllAttributes(T *list)
  {
    list->Add(mSomeEnum);
  }

  chip::Span<const EmberAfAttributeMetadata> GetAllAttributes() override
  {
    static constexpr const EmberAfAttributeMetadata kAllAttributes[] = {
      { 10, ZCL_ENUM8_ATTRIBUTE_TYPE, 1, ATTRIBUTE_MASK_WRITABLE | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() },
    };
    return chip::Span<const EmberAfAttributeMetadata>(kAllAttributes);
  }


  Attribute<10, ATTRIBUTE_MASK_WRITABLE, PrimitiveType<uint8_t, 1, ZCL_ENUM8_ATTRIBUTE_TYPE>> mSomeEnum;
};

struct ThirdAccess : public CommonAttributeAccessInterface
{
  ThirdAccess() : CommonAttributeAccessInterface(chip::Optional<chip::EndpointId>(), ThirdCluster::kClusterId) {}

  ThirdCluster* GetCluster(const chip::app::ConcreteClusterPath & aPath)
  {
    CommonCluster * cluster = FindCluster(aPath);
    return cluster ? static_cast<ThirdCluster*>(cluster) : nullptr;
  }

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override
  {
    auto * c = GetCluster(aPath);
    if (!c)
      return CHIP_ERROR_NOT_IMPLEMENTED;

    switch(aPath.mAttributeId) {
    case 10:
      return c->mSomeEnum.Read(aPath, aEncoder);
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