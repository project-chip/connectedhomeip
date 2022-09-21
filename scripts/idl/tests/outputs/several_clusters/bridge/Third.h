#pragma once

#include "BridgeGlobalStructs.h"

namespace clusters {
struct ThirdCluster : public GeneratedCluster
{

  ThirdCluster() :
      mSomeEnum(chip::CharSpan("someEnum"), 10, ATTRIBUTE_MASK_WRITABLE, ZCL_ENUM8_ATTRIBUTE_TYPE, 1)
  {
  }

  chip::ClusterId GetClusterId() override { return 3; }

  std::vector<AttributeInterface*> GetAttributes() override
  {
    return std::vector<AttributeInterface*>({
      static_cast<AttributeInterface*>(&mSomeEnum),
    });
  }


  Attribute<uint8_t> mSomeEnum;
};

}