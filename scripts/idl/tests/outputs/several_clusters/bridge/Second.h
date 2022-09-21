#pragma once

#include "BridgeGlobalStructs.h"

namespace clusters {
struct SecondCluster : public GeneratedCluster
{

  SecondCluster() :
      mSomeBytes(chip::CharSpan("someBytes"), 123, 0, ZCL_OCTET_STRING_ATTRIBUTE_TYPE, 32)
  {
  }

  chip::ClusterId GetClusterId() override { return 2; }

  std::vector<AttributeInterface*> GetAttributes() override
  {
    return std::vector<AttributeInterface*>({
      static_cast<AttributeInterface*>(&mSomeBytes),
    });
  }


  Attribute<std::string> mSomeBytes;
};

}