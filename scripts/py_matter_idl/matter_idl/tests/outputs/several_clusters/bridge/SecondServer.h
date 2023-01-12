#pragma once

#include "BridgeGlobalStructs.h"
#include "third_party/connectedhomeip/examples/dynamic-bridge-app/linux/include/GeneratedClusters.h"

namespace clusters {
struct SecondCluster : public GeneratedCluster
{

  SecondCluster() :
      mSomeBytes(chip::CharSpan("someBytes"), 123, 0, ZCL_OCTET_STRING_ATTRIBUTE_TYPE, 32)
  {
  }

  static constexpr uint32_t kClusterId =2;
  chip::ClusterId GetClusterId() override { return kClusterId; }

  std::vector<AttributeInterface*> GetAttributes() override
  {
    return std::vector<AttributeInterface*>({
      static_cast<AttributeInterface*>(&mSomeBytes),
    });
  }


  Attribute<std::string> mSomeBytes;
};

}
