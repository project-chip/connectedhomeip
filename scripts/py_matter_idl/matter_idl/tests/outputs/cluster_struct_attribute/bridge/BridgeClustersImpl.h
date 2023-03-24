#pragma once

#include <new>
#include "bridge/DemoCluster.h"

namespace clusters {

struct ClusterInfo
{
  chip::ClusterId id;
  const char *name;
  uint16_t size;
  GeneratedCluster* (*ctor)(void*);
} static const kKnownClusters[] = {

  {
    10,
    "DemoCluster",
    sizeof(DemoClusterCluster),
    [](void *mem) -> GeneratedCluster* {
      return new(mem) DemoClusterCluster();
    },
  },
};

}
