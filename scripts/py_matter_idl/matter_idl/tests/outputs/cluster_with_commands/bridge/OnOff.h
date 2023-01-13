#pragma once

#include "BridgeGlobalStructs.h"
#include "GeneratedClusters.h"

namespace clusters {
struct OnOffCluster : public GeneratedCluster
{

  OnOffCluster() :
      mOnOff(chip::CharSpan("onOff"), 0, 0 | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZCL_BOOLEAN_ATTRIBUTE_TYPE, 1),
      mFeatureMap(chip::CharSpan("featureMap"), 65532, 0 | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZCL_BITMAP32_ATTRIBUTE_TYPE, 4),
      mClusterRevision(chip::CharSpan("clusterRevision"), 65533, 0 | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZCL_INT16U_ATTRIBUTE_TYPE, 2, ZCL_ON_OFF_CLUSTER_REVISION)
  {
  }

  static constexpr uint32_t kClusterId =6;
  chip::ClusterId GetClusterId() override { return kClusterId; }

  std::vector<AttributeInterface*> GetAttributes() override
  {
    return std::vector<AttributeInterface*>({
      static_cast<AttributeInterface*>(&mOnOff),
      static_cast<AttributeInterface*>(&mFeatureMap),
      static_cast<AttributeInterface*>(&mClusterRevision),
    });
  }


  Attribute<bool> mOnOff;
  Attribute<uint32_t> mFeatureMap;
  Attribute<uint16_t> mClusterRevision;
  static const chip::CommandId mIncomingCommandList[];
  const chip::CommandId * GetIncomingCommandList() override
  {
    return mIncomingCommandList;
  }
};
#ifndef ON_OFF_CLUSTER_INCOMING_COMMANDS
#define ON_OFF_CLUSTER_INCOMING_COMMANDS
const chip::CommandId OnOffCluster::mIncomingCommandList[] = {
  0,
  1,
  2,
  chip::kInvalidCommandId
};
#endif

}
