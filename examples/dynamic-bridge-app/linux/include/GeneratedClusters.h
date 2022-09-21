#pragma once

#include <app/AttributeAccessInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>

#include <stdbool.h>
#include <stdint.h>

#include <functional>

#include "Attribute.h"
#include "Clusters.h"
#include "DataModel.h"

struct CommonAttributeAccessInterface : public chip::app::AttributeAccessInterface
{
    using chip::app::AttributeAccessInterface::AttributeAccessInterface;

    // Find a cluster given a specific endpoint/cluster. Returns nullptr if no such
    // cluster exists at that path.
    static CommonCluster * FindCluster(const chip::app::ConcreteClusterPath & path);

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

    void OnListWriteBegin(const chip::app::ConcreteAttributePath & aPath) override;
    void OnListWriteEnd(const chip::app::ConcreteAttributePath & aPath, bool aWriteWasSuccessful) override;
};

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

#include "bridge/BridgeClustersImpl.h"

/*

namespace clusters {
struct BridgedDeviceBasicCluster : public CommonCluster
{
    static constexpr chip::ClusterId kClusterId = ZCL_BRIDGED_DEVICE_BASIC_CLUSTER_ID;

    BridgedDeviceBasicCluster();

    chip::ClusterId GetClusterId() override { return kClusterId; }
    CHIP_ERROR WriteFromBridge(const chip::app::ConcreteDataAttributePath & aPath,
                               chip::app::AttributeValueDecoder & aDecoder) override;

    chip::Span<const EmberAfAttributeMetadata> GetAllAttributes() override
    {
        static constexpr EmberAfAttributeMetadata kAllAttributes[] = {
            { ZCL_REACHABLE_ATTRIBUTE_ID, ZCL_BOOLEAN_ATTRIBUTE_TYPE, 1, 0, 0u },
        };
        return chip::Span<const EmberAfAttributeMetadata>(kAllAttributes);
    }

    Attribute<ZCL_REACHABLE_ATTRIBUTE_ID, 0, ZCL_BOOLEAN_ATTRIBUTE_TYPE, 1, uint8_t, false> mReachable;
};
} // namespace clusters
*/
