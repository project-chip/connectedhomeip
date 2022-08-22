#pragma once

#include <new>
#include "bridge/DemoCluster.h"

namespace clusters {

struct ClusterInfo
{
  chip::ClusterId id;
  const char *name;
  uint16_t size;
  CommonCluster* (*ctor)(void*);
} static const kKnownClusters[] = {

  {
    18,
    "DemoCluster",
    sizeof(DemoClusterCluster),
    [](void *mem) -> CommonCluster* {
      return new(mem) DemoClusterCluster();
    },
  },
};

inline void BridgeRegisterAllAttributeOverrides()
{

  static DemoClusterAccess DemoCluster;
  registerAttributeAccessOverride(&DemoCluster);
}

struct AttrInfo
{
  chip::ClusterId cluster;
  chip::AttributeId attr;
  const char *name;
} static const kKnownAttributes[] = {

  { 18, 32, "SingleLabel" },
  { 18, 33, "SomeLabels" },
};

}