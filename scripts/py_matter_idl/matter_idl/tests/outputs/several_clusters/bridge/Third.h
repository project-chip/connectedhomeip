#pragma once

#include "BridgeGlobalStructs.h"
#include "GeneratedClusters.h"

namespace clusters {
struct ThirdCluster : public GeneratedCluster
{

  ThirdCluster() :
      mSomeEnum(chip::CharSpan("someEnum"), 10, ATTRIBUTE_MASK_WRITABLE | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZCL_ENUM8_ATTRIBUTE_TYPE, 1),
      mClusterRevision(chip::CharSpan("clusterRevision"), 65533, 0 | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZCL_INT16U_ATTRIBUTE_TYPE, 2, ZCL_THIRD_CLUSTER_REVISION)
  {
  }

  static constexpr uint32_t kClusterId =3;
  chip::ClusterId GetClusterId() override { return kClusterId; }

  std::vector<AttributeInterface*> GetAttributes() override
  {
    return std::vector<AttributeInterface*>({
      static_cast<AttributeInterface*>(&mSomeEnum),
      static_cast<AttributeInterface*>(&mClusterRevision),
    });
  }


  Attribute<uint8_t> mSomeEnum;
  Attribute<uint16_t> mClusterRevision;
};

}
