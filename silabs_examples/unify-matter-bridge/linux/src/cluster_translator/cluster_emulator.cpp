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

uint32_t ClusterEmulator::read_cluster_revision(const ConcreteReadAttributePath & aPath) const
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

uint32_t ClusterEmulator::read_feature_map_revision(const ConcreteReadAttributePath & aPath) const
{
    sl_log_debug(LOG_TAG, "Reading feature map for cluster %d", aPath.mClusterId);
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
        sl_log_debug(LOG_TAG, "Returning feature map for OnOff %d", ON_OFF_LIGHTING_FEATURE_MAP_MASK);
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
    const node_state_monitor::cluster & unify_cluster,
    matter_cluster_builder & cluster_builder)
{
    // We always need to add the feature map and cluster
    cluster_builder.attributes.push_back(EmberAfAttributeMetadata{ ZCL_FEATURE_MAP_SERVER_ATTRIBUTE_ID, ZCL_BITMAP32_ATTRIBUTE_TYPE,
                                                                   4, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() });
    cluster_builder.attributes.push_back(EmberAfAttributeMetadata{ ZCL_CLUSTER_REVISION_SERVER_ATTRIBUTE_ID,
                                                                   ZCL_INT16U_ATTRIBUTE_TYPE, 2,
                                                                   ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() });
    
    // Add emulation for commands and attributes for the cluster
    auto it = cluster_emulators_string_map.find(unify_cluster.cluster_name);
    if (it != cluster_emulators_string_map.end()) {
        auto emulated_result = it->second->emulate(unify_cluster, cluster_builder, cluster_emulators_attribute_id_map, cluster_emulators_command_id_map);
        if (emulated_result != CHIP_NO_ERROR) {
            sl_log_error(LOG_TAG, "Failed to add emulated commands and attributes for cluster %s", unify_cluster.cluster_name.c_str());
        }
    }
}

bool ClusterEmulator::is_command_emulated(const ConcreteCommandPath & cPath) const
{   
    auto it = cluster_emulators_command_id_map.find(cPath.mClusterId);
    if (it != cluster_emulators_command_id_map.end())
    {
        return it->second.find(cPath.mCommandId) != it->second.end();
    }
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

    auto it = cluster_emulators_attribute_id_map.find(aPath.mClusterId);
    if (it != cluster_emulators_attribute_id_map.end())
    {
        return it->second.find(aPath.mAttributeId) != it->second.end();
    }

    return false;
}

CHIP_ERROR ClusterEmulator::invoke_command(CommandHandlerInterface::HandlerContext & handlerContext) const
{
    if (is_command_emulated(handlerContext.mRequestPath))
    {
        return cluster_emulators_command_id_map.at(handlerContext.mRequestPath.mClusterId).at(handlerContext.mRequestPath.mCommandId)->command(handlerContext);
    }

    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ClusterEmulator::read_attribute(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    sl_log_debug(LOG_TAG, "Reading attribute %d", aPath.mAttributeId);
    switch (aPath.mAttributeId)
    {
    case ZCL_CLUSTER_REVISION_SERVER_ATTRIBUTE_ID: // Cluster Revision
        return aEncoder.Encode(this->read_cluster_revision(aPath));
    case ZCL_FEATURE_MAP_SERVER_ATTRIBUTE_ID: // FeatureMap
        return aEncoder.Encode(this->read_feature_map_revision(aPath));
    case 0xFFFE: // EventList
        return aEncoder.Encode(0);
    default:;
        ;
    }
    
    sl_log_debug(LOG_TAG, "Cluster specific attribute emulation attribute id %d for cluster id", aPath.mAttributeId, aPath.mClusterId);
    if (is_attribute_emulated(aPath))
    {
        return cluster_emulators_attribute_id_map.at(aPath.mClusterId).at(aPath.mAttributeId)->read_attribute(aPath, aEncoder);
    }

    return CHIP_ERROR_INVALID_ARGUMENT;
}

CHIP_ERROR ClusterEmulator::write_attribute(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) const
{
    if (is_attribute_emulated(aPath))
    {
        return cluster_emulators_attribute_id_map.at(aPath.mClusterId).at(aPath.mAttributeId)->write_attribute(aPath, aDecoder);
    }

    return CHIP_ERROR_NOT_IMPLEMENTED;
}
} // namespace unify::matter_bridge
