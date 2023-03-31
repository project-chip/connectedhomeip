#pragma once

#include "BridgeGlobalStructs.h"
#include "GeneratedClusters.h"

namespace clusters {
struct ThirdCluster : public GeneratedCluster
{

  ThirdCluster() :
      mSomeEnum(chip::CharSpan("someEnum"), 10, ATTRIBUTE_MASK_WRITABLE | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZCL_ENUM8_ATTRIBUTE_TYPE, 1),
      mOptions(chip::CharSpan("options"), 20, ATTRIBUTE_MASK_WRITABLE | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZCL_BITMAP8_ATTRIBUTE_TYPE, 1)
  {
  }

  static constexpr uint32_t kClusterId =3;
  chip::ClusterId GetClusterId() override { return kClusterId; }

  std::vector<AttributeInterface*> GetAttributes() override
  {
    return std::vector<AttributeInterface*>({
      static_cast<AttributeInterface*>(&mSomeEnum),
      static_cast<AttributeInterface*>(&mOptions),
    });
  }


  Attribute<uint8_t> mSomeEnum;
  Attribute<uint8_t> mOptions;
};

}
