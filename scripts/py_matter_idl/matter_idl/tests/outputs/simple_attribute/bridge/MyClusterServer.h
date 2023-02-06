#pragma once

#include "BridgeGlobalStructs.h"
#include "GeneratedClusters.h"

namespace clusters {
struct MyClusterCluster : public GeneratedCluster
{

  MyClusterCluster() :
      mClusterAttr(chip::CharSpan("clusterAttr"), 1, ATTRIBUTE_MASK_WRITABLE | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZCL_INT16U_ATTRIBUTE_TYPE, 2)
  {
  }

  static constexpr uint32_t kClusterId =123;
  chip::ClusterId GetClusterId() override { return kClusterId; }

  std::vector<AttributeInterface*> GetAttributes() override
  {
    return std::vector<AttributeInterface*>({
      static_cast<AttributeInterface*>(&mClusterAttr),
    });
  }


  Attribute<uint16_t> mClusterAttr;
};

}
