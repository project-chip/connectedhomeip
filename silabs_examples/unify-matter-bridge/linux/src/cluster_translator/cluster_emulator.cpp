/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#include "cluster_emulator.hpp"
#include "cluster_revision_table.hpp"
#include "sl_log.h"
#include <attribute_state_cache.hpp>
#define LOG_TAG "cluster_emulator"

namespace unify::matter_bridge {

#define ON_OFF_LIGHTING_FEATURE_MAP_MASK 0x01
#define LEVEL_ONOFF_DEPENDENCY_FEATURE_MAP_MASK 0x01
#define LEVEL_LIGHTING_FEATURE_MAP_MASK 0x02

using namespace chip::app;
using namespace chip::app::Clusters;

static uint32_t read_cluster_revision(const ConcreteReadAttributePath & aPath)
{
    if (zap_cluster_revisions.count(aPath.mClusterId))
    {
        return zap_cluster_revisions.at(aPath.mClusterId);
    }
    else
    {
        return 0;
    }
}

static uint32_t read_feature_map_revision(const ConcreteReadAttributePath & aPath)
{
    switch (aPath.mClusterId)
    {
    case ColorControl::Id: {
        ColorControl::Attributes::ColorCapabilities::TypeInfo::Type colorControlCapabilities;
        attribute_state_cache & cache = attribute_state_cache::get_instance();
        if (cache.get<ColorControl::Attributes::ColorCapabilities::TypeInfo::Type>(aPath, colorControlCapabilities))
        {
            return static_cast<uint32_t>(colorControlCapabilities);
        }
        else
        {
            sl_log_warning(LOG_TAG, "Failed to read ColorCapabilities, setting feature map to HueSaturationSupported");
            return 1;
        }
    }
    break;
    case OnOff::Id:
        return ON_OFF_LIGHTING_FEATURE_MAP_MASK;
    case LevelControl::Id:
        /// Check if OnOff is supported
        if (emberAfFindServerCluster(aPath.mEndpointId, OnOff::Id))
        {
            return (LEVEL_ONOFF_DEPENDENCY_FEATURE_MAP_MASK | LEVEL_LIGHTING_FEATURE_MAP_MASK);
        }
        break;
    }
    return 0;
}

void ClusterEmulator::add_emulated_commands_and_attributes(
    const std::unordered_map<std::string, node_state_monitor::cluster> & unify_clusters,
    matter_cluster_builder & cluster_builder) const
{
    // We always need to add the feature map and cluster
    cluster_builder.attributes.push_back(EmberAfAttributeMetadata{ ZCL_FEATURE_MAP_SERVER_ATTRIBUTE_ID, ZCL_BITMAP32_ATTRIBUTE_TYPE,
                                                                   4, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() });
    cluster_builder.attributes.push_back(EmberAfAttributeMetadata{ ZCL_CLUSTER_REVISION_SERVER_ATTRIBUTE_ID,
                                                                   ZCL_INT16U_ATTRIBUTE_TYPE, 2,
                                                                   ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() });
}

bool ClusterEmulator::is_command_emulated(const ConcreteCommandPath &) const
{
    return false;
}

bool ClusterEmulator::is_attribute_emulated(const ConcreteAttributePath & aPath) const
{
    // Global attributes per Core Spec chapter 7.13
    switch (aPath.mAttributeId)
    {
    case 0xFFFD: // Cluster Revision
    case 0xFFFC: // FeatureMap
    case 0xFFFE: // EventList
        return true;
    default:;
        ;
    }

    return false;
}

CHIP_ERROR ClusterEmulator::invoke_command(CommandHandlerInterface::HandlerContext & handlerContext) const
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ClusterEmulator::read_attribute(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) const
{
    switch (aPath.mAttributeId)
    {
    case 0xFFFD: // Cluster Revision
        return aEncoder.Encode(read_cluster_revision(aPath));
    case 0xFFFC: // FeatureMap
        return aEncoder.Encode(read_feature_map_revision(aPath));
    case 0xFFFE: // EventList
        return aEncoder.Encode(0);
    default:;
        ;
    }

    return CHIP_ERROR_INVALID_ARGUMENT;
}

CHIP_ERROR ClusterEmulator::write_attribute(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) const
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
} // namespace unify::matter_bridge
