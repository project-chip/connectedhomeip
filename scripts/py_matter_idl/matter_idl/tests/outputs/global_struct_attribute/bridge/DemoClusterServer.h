#pragma once

#include "BridgeGlobalStructs.h"
#include "GeneratedClusters.h"

namespace clusters {
struct DemoClusterCluster : public GeneratedCluster
{

  DemoClusterCluster() :
      mSingleLabel(chip::CharSpan("singleLabel"), 32, ATTRIBUTE_MASK_WRITABLE | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZCL_STRUCT_ATTRIBUTE_TYPE, sizeof(LabelStruct)),
      mSomeLabels(chip::CharSpan("someLabels"), 33, ATTRIBUTE_MASK_WRITABLE | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZCL_ARRAY_ATTRIBUTE_TYPE, sizeof(LabelStruct))
  {
  }

  static constexpr uint32_t kClusterId =18;
  chip::ClusterId GetClusterId() override { return kClusterId; }

  std::vector<AttributeInterface*> GetAttributes() override
  {
    return std::vector<AttributeInterface*>({
      static_cast<AttributeInterface*>(&mSingleLabel),
      static_cast<AttributeInterface*>(&mSomeLabels),
    });
  }


  Attribute<LabelStruct> mSingleLabel;
  ListAttribute<std::vector<LabelStruct>> mSomeLabels;
};

}
