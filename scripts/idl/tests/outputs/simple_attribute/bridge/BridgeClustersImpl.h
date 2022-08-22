#pragma once

#include <new>
#include "bridge/MyCluster.h"

namespace clusters {

struct ClusterInfo
{
  chip::ClusterId id;
  const char *name;
  uint16_t size;
  CommonCluster* (*ctor)(void*);
} static const kKnownClusters[] = {

  {
    123,
    "MyCluster",
    sizeof(MyClusterCluster),
    [](void *mem) -> CommonCluster* {
      return new(mem) MyClusterCluster();
    },
  },
};

inline void BridgeRegisterAllAttributeOverrides()
{

  static MyClusterAccess MyCluster;
  registerAttributeAccessOverride(&MyCluster);
}

struct AttrInfo
{
  chip::ClusterId cluster;
  chip::AttributeId attr;
  const char *name;
} static const kKnownAttributes[] = {

  { 123, 1, "ClusterAttr" },
};

}