#pragma once

#include "BridgeGlobalStructs.h"

namespace clusters {
struct DemoClusterCluster : public CommonCluster
{


  static constexpr chip::ClusterId kClusterId = 18;

  chip::ClusterId GetClusterId() override { return kClusterId; }

  CHIP_ERROR WriteFromBridge(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override
  {
    switch(aPath.mAttributeId)
    {
    case 32:
      return mSingleLabel.Write(aPath, aDecoder);
    case 33:
      return mSomeLabels.Write(aPath, aDecoder);
    default:
      return CHIP_ERROR_NOT_IMPLEMENTED;
    }
  }

  chip::Span<const EmberAfAttributeMetadata> GetAllAttributes() override
  {
    static constexpr const EmberAfAttributeMetadata kAllAttributes[] = {
      { 32, ZCL_STRUCT_ATTRIBUTE_TYPE, sizeof(LabelStruct), ATTRIBUTE_MASK_WRITABLE | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() },
      { 33, ZCL_ARRAY_ATTRIBUTE_TYPE, sizeof(LabelStruct) * 1 + 2, ATTRIBUTE_MASK_WRITABLE | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() },
    };
    return chip::Span<const EmberAfAttributeMetadata>(kAllAttributes);
  }


  Attribute<32, ATTRIBUTE_MASK_WRITABLE, ZCL_STRUCT_ATTRIBUTE_TYPE, sizeof(LabelStruct), LabelStruct, false> mSingleLabel;
  ArrayAttribute<1, false, 33, ATTRIBUTE_MASK_WRITABLE, ZCL_ARRAY_ATTRIBUTE_TYPE, sizeof(LabelStruct) * 1 + 2, LabelStruct, false> mSomeLabels;
};

struct DemoClusterAccess : public CommonAttributeAccessInterface
{
  DemoClusterAccess() : CommonAttributeAccessInterface(chip::Optional<chip::EndpointId>(), DemoClusterCluster::kClusterId) {}

  DemoClusterCluster* GetCluster(const chip::app::ConcreteClusterPath & aPath)
  {
    CommonCluster * cluster = FindCluster(aPath);
    return cluster ? static_cast<DemoClusterCluster*>(cluster) : nullptr;
  }

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override
  {
    auto * c = GetCluster(aPath);
    if (!c)
      return CHIP_ERROR_NOT_IMPLEMENTED;

    switch(aPath.mAttributeId) {
    case 32:
      return c->mSingleLabel.Read(aPath, aEncoder);
    case 33:
      return c->mSomeLabels.Read(aPath, aEncoder);
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
    case 33:
      c->mSomeLabels.ListWriteBegin(aPath);
      return;
    }
  }

  void OnListWriteEnd(const chip::app::ConcreteAttributePath & aPath, bool aWriteWasSuccessful) override
  {
    auto * c = GetCluster(aPath);
    if (!c)
      return;

    switch(aPath.mAttributeId) {
    case 33:
      c->mSomeLabels.ListWriteEnd(aPath, aWriteWasSuccessful);
      return;
    }
  }
};

}