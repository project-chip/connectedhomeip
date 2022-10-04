#pragma once

#include <app/AttributeAccessInterface.h>
#include <app/util/attribute-storage.h>

#include <vector>

#include "data-model/Attribute.h"

// Required for default initialization of cluster revisions.
static constexpr uint16_t ZCL_DESCRIPTOR_CLUSTER_REVISION              = 1;
static constexpr uint16_t ZCL_BRIDGED_DEVICE_BASIC_CLUSTER_REVISION    = 1;
static constexpr uint16_t ZCL_FIXED_LABEL_CLUSTER_REVISION             = 1;
static constexpr uint16_t ZCL_ON_OFF_CLUSTER_REVISION                  = 4;
static constexpr uint16_t ZCL_SWITCH_CLUSTER_REVISION                  = 1;
static constexpr uint16_t ZCL_LEVEL_CONTROL_CLUSTER_REVISION           = 1;
static constexpr uint16_t ZCL_TEMPERATURE_MEASUREMENT_CLUSTER_REVISION = 1;

class GeneratedCluster
{
public:
    virtual ~GeneratedCluster() = default;
    // Get the ID for this cluster.
    virtual chip::ClusterId GetClusterId() = 0;
    // Gets the list of available attributes for this cluster.
    virtual std::vector<AttributeInterface *> GetAttributes() = 0;
};
