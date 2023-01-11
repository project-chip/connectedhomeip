#pragma once

#include "bridge/OnOff.h"
#include <new>

namespace clusters {

struct ClusterInfo
{
    chip::ClusterId id;
    const char * name;
    uint16_t size;
    GeneratedCluster * (*ctor)(void *);
} static const kKnownClusters[] = {

    {
        6,
        "OnOff",
        sizeof(OnOffCluster),
        [](void * mem) -> GeneratedCluster * { return new (mem) OnOffCluster(); },
    },
};

} // namespace clusters
