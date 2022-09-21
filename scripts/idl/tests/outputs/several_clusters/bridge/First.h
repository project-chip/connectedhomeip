#pragma once

#include "BridgeGlobalStructs.h"

namespace clusters {
struct FirstCluster : public GeneratedCluster
{

  FirstCluster() :
      mSomeInteger(chip::CharSpan("someInteger"), 1, ATTRIBUTE_MASK_WRITABLE, ZCL_INT16U_ATTRIBUTE_TYPE, 2)
  {
  }

  chip::ClusterId GetClusterId() override { return 1; }

  std::vector<AttributeInterface*> GetAttributes() override
  {
    return std::vector<AttributeInterface*>({
      static_cast<AttributeInterface*>(&mSomeInteger),
    });
  }


  Attribute<uint16_t> mSomeInteger;
};

}