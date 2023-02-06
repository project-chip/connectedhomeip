#pragma once

#include <new>
#include "bridge/OnOff.h"

namespace clusters {

struct ClusterInfo
{
  chip::ClusterId id;
  const char *name;
  uint16_t size;
  GeneratedCluster* (*ctor)(void*);
} static const kKnownClusters[] = {

  {
    6,
    "OnOff",
    sizeof(OnOffCluster),
    [](void *mem) -> GeneratedCluster* {
      return new(mem) OnOffCluster();
    },
  },
};

}
