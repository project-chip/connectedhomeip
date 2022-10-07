#pragma once

#include "BridgeGlobalStructs.h"
#include "third_party/connectedhomeip/examples/dynamic-bridge-app/linux/include/GeneratedClusters.h"
#include "third_party/connectedhomeip/examples/dynamic-bridge-app/linux/include/data-model/DataModel.h"

namespace clusters {
struct DemoClusterCluster : public GeneratedCluster
{

  DemoClusterCluster() :
      mSingleFailSafe(chip::CharSpan("singleFailSafe"), 5, ATTRIBUTE_MASK_WRITABLE, ZCL_STRUCT_ATTRIBUTE_TYPE, sizeof(ArmFailSafeRequest)),
      mArmFailsafes(chip::CharSpan("armFailsafes"), 100, ATTRIBUTE_MASK_WRITABLE, ZCL_ARRAY_ATTRIBUTE_TYPE, sizeof(ArmFailSafeRequest))
  {
  }

  static constexpr uint32_t kClusterId =10;
  chip::ClusterId GetClusterId() override { return kClusterId; }

  std::vector<AttributeInterface*> GetAttributes() override
  {
    return std::vector<AttributeInterface*>({
      static_cast<AttributeInterface*>(&mSingleFailSafe),
      static_cast<AttributeInterface*>(&mArmFailsafes),
    });
  }


  Attribute<ArmFailSafeRequest> mSingleFailSafe;
  ListAttribute<::chip::app::DataModel::ListContainer<std::vector<ArmFailSafeRequest>>> mArmFailsafes;
};

}