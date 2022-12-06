/*******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include "feature_map_attr_override.hpp"
#include "sl_log.h"
#include <attribute_state_cache.hpp>

using namespace chip;
using namespace chip::app;
using namespace unify::matter_bridge;
#define LOG_TAG "attribute_translator_override"

#define ON_OFF_LIGHTING_FEATURE_MAP_MASK 0x01
#define LEVEL_ONOFF_DEPENDENCY_FEATURE_MAP_MASK 0x01
#define LEVEL_LIGHTING_FEATURE_MAP_MASK 0x02

uint32_t ColorControlAttributeAccessOverride::get_feature_map_settings(const ConcreteReadAttributePath & aPath)
{
    uint32_t feature_map_value = 0;
    chip::app::Clusters::ColorControl::Attributes::ColorCapabilities::TypeInfo::Type colorControlCapabilities;
    ConcreteAttributePath cc_atr_path = ConcreteAttributePath(aPath.mEndpointId, chip::app::Clusters::ColorControl::Id,
                                                              chip::app::Clusters::ColorControl::Attributes::ColorCapabilities::Id);

    attribute_state_cache & cache = attribute_state_cache::get_instance();
    if (cache.get<chip::app::Clusters::ColorControl::Attributes::ColorCapabilities::TypeInfo::Type>(cc_atr_path,
                                                                                                    colorControlCapabilities))
    {
        feature_map_value = static_cast<uint32_t>(colorControlCapabilities);
    }
    else
    {
        sl_log_warning(LOG_TAG, "Failed to read ColorCapabilities, setting feature map to HueSaturationSupported");
        feature_map_value = 1;
    }

    return feature_map_value;
}

uint32_t OnOffAttributeAccessOverride::get_feature_map_settings(const ConcreteReadAttributePath & aPath)
{
    uint32_t feature_map_value             = 0;
    std::set<chip::ClusterId> cluster_list = m_node_state_monitor.get_supported_cluster(aPath.mEndpointId);
    auto level_supported                   = cluster_list.find(chip::app::Clusters::LevelControl::Id);

    if (level_supported != cluster_list.end())
    {
        feature_map_value |= ON_OFF_LIGHTING_FEATURE_MAP_MASK;
    }

    return feature_map_value;
}

uint32_t LevelControlAttributeAccessOverride::get_feature_map_settings(const ConcreteReadAttributePath & aPath)
{
    uint32_t feature_map_value             = 0;
    std::set<chip::ClusterId> cluster_list = m_node_state_monitor.get_supported_cluster(aPath.mEndpointId);
    auto on_off_supported                  = cluster_list.find(chip::app::Clusters::OnOff::Id);

    if (on_off_supported != cluster_list.end())
    {
        feature_map_value |= LEVEL_ONOFF_DEPENDENCY_FEATURE_MAP_MASK;
        feature_map_value |= LEVEL_LIGHTING_FEATURE_MAP_MASK;
    }

    return feature_map_value;
}
