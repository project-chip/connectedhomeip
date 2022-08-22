#pragma once

#include <new>
#include "bridge/First.h"
#include "bridge/Second.h"
#include "bridge/Third.h"

namespace clusters {

struct ClusterInfo
{
  chip::ClusterId id;
  const char *name;
  uint16_t size;
  CommonCluster* (*ctor)(void*);
} static const kKnownClusters[] = {

  {
    1,
    "First",
    sizeof(FirstCluster),
    [](void *mem) -> CommonCluster* {
      return new(mem) FirstCluster();
    },
  },
  {
    2,
    "Second",
    sizeof(SecondCluster),
    [](void *mem) -> CommonCluster* {
      return new(mem) SecondCluster();
    },
  },
  {
    3,
    "Third",
    sizeof(ThirdCluster),
    [](void *mem) -> CommonCluster* {
      return new(mem) ThirdCluster();
    },
  },
};

inline void BridgeRegisterAllAttributeOverrides()
{

  static FirstAccess First;
  registerAttributeAccessOverride(&First);
  static SecondAccess Second;
  registerAttributeAccessOverride(&Second);
  static ThirdAccess Third;
  registerAttributeAccessOverride(&Third);
}

struct AttrInfo
{
  chip::ClusterId cluster;
  chip::AttributeId attr;
  const char *name;
} static const kKnownAttributes[] = {

  { 1, 1, "SomeInteger" },
  { 2, 123, "SomeBytes" },
  { 3, 10, "SomeEnum" },
};

}