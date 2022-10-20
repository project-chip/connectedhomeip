#pragma once

#include <new>
#include "bridge/First.h"
#include "bridge/Second.h"
#include "bridge/Third.h"
#include "bridge/Third.h"

namespace clusters {

struct ClusterInfo
{
  chip::ClusterId id;
  const char *name;
  uint16_t size;
  GeneratedCluster* (*ctor)(void*);
} static const kKnownClusters[] = {

  {
    1,
    "First",
    sizeof(FirstCluster),
    [](void *mem) -> GeneratedCluster* {
      return new(mem) FirstCluster();
    },
  },
  {
    2,
    "Second",
    sizeof(SecondCluster),
    [](void *mem) -> GeneratedCluster* {
      return new(mem) SecondCluster();
    },
  },
  {
    3,
    "Third",
    sizeof(ThirdCluster),
    [](void *mem) -> GeneratedCluster* {
      return new(mem) ThirdCluster();
    },
  },
  {
    3,
    "Third",
    sizeof(ThirdCluster),
    [](void *mem) -> GeneratedCluster* {
      return new(mem) ThirdCluster();
    },
  },
};

}
