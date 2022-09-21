#pragma once

#include "BridgeGlobalStructs.h"

namespace clusters {
struct MyClusterCluster : public GeneratedCluster
{

  MyClusterCluster() :
      mClusterAttr(chip::CharSpan("clusterAttr"), 1, ATTRIBUTE_MASK_WRITABLE, ZCL_INT16U_ATTRIBUTE_TYPE, 2)
  {
  }

  chip::ClusterId GetClusterId() override { return 123; }

  std::vector<AttributeInterface*> GetAttributes() override
  {
    return std::vector<AttributeInterface*>({
      static_cast<AttributeInterface*>(&mClusterAttr),
    });
  }


  Attribute<uint16_t> mClusterAttr;
};

}