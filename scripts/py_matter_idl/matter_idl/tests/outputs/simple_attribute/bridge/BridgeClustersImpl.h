#pragma once

#include <new>
#include "bridge/MyCluster.h"

namespace clusters {

struct ClusterInfo
{
  chip::ClusterId id;
  const char *name;
  uint16_t size;
  GeneratedCluster* (*ctor)(void*);
} static const kKnownClusters[] = {

  {
    123,
    "MyCluster",
    sizeof(MyClusterCluster),
    [](void *mem) -> GeneratedCluster* {
      return new(mem) MyClusterCluster();
    },
  },
};

}
