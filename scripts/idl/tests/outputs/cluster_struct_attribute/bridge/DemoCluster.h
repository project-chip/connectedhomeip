#pragma once

#include "BridgeGlobalStructs.h"

namespace clusters {
struct DemoClusterCluster : public GeneratedCluster
{

  DemoClusterCluster() :
      mSingleFailSafe(chip::CharSpan("singleFailSafe"), 5, ATTRIBUTE_MASK_WRITABLE, ZCL_STRUCT_ATTRIBUTE_TYPE, sizeof(ArmFailSafeRequest)),
      mArmFailsafes(chip::CharSpan("armFailsafes"), 100, ATTRIBUTE_MASK_WRITABLE, ZCL_ARRAY_ATTRIBUTE_TYPE, sizeof(ArmFailSafeRequest))
  {
  }

  chip::ClusterId GetClusterId() override { return 10; }

  std::vector<AttributeInterface*> GetAttributes() override
  {
    return std::vector<AttributeInterface*>({
      static_cast<AttributeInterface*>(&mSingleFailSafe),
      static_cast<AttributeInterface*>(&mArmFailsafes),
    });
  }


  Attribute<ArmFailSafeRequest> mSingleFailSafe;
  ListAttribute<std::vector<ArmFailSafeRequest>> mArmFailsafes;
};

}