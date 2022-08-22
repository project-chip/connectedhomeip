#pragma once

#include "BridgeGlobalStructs.h"

namespace clusters {
struct MyClusterCluster : public CommonCluster
{


  static constexpr chip::ClusterId kClusterId = 123;

  chip::ClusterId GetClusterId() override { return kClusterId; }

  CHIP_ERROR WriteFromBridge(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override
  {
    switch(aPath.mAttributeId)
    {
    case 1:
      return mClusterAttr.Write(aPath, aDecoder);
    default:
      return CHIP_ERROR_NOT_IMPLEMENTED;
    }
  }

  template<typename T>
  void AddAllAttributes(T *list)
  {
    list->Add(mClusterAttr);
  }

  chip::Span<const EmberAfAttributeMetadata> GetAllAttributes() override
  {
    static constexpr const EmberAfAttributeMetadata kAllAttributes[] = {
      { 1, ZCL_INT16U_ATTRIBUTE_TYPE, 2, ATTRIBUTE_MASK_WRITABLE | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() },
    };
    return chip::Span<const EmberAfAttributeMetadata>(kAllAttributes);
  }


  Attribute<1, ATTRIBUTE_MASK_WRITABLE, PrimitiveType<uint16_t, 2, ZCL_INT16U_ATTRIBUTE_TYPE>> mClusterAttr;
};

struct MyClusterAccess : public CommonAttributeAccessInterface
{
  MyClusterAccess() : CommonAttributeAccessInterface(chip::Optional<chip::EndpointId>(), MyClusterCluster::kClusterId) {}

  MyClusterCluster* GetCluster(const chip::app::ConcreteClusterPath & aPath)
  {
    CommonCluster * cluster = FindCluster(aPath);
    return cluster ? static_cast<MyClusterCluster*>(cluster) : nullptr;
  }

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override
  {
    auto * c = GetCluster(aPath);
    if (!c)
      return CHIP_ERROR_NOT_IMPLEMENTED;

    switch(aPath.mAttributeId) {
    case 1:
      return c->mClusterAttr.Read(aPath, aEncoder);
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