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
// Own header
#include "matter_device_translator.hpp"

// Application library
#include "matter_device_mapper.inc"
#include "matter_device_type_selection.hpp"
#include "matter_device_types_clusters_list.inc"
#include "matter_id_model.inc"

// Matter library
#include "matter.h"

// Unify components
#include "sl_log.h"

// Standard library
#include <cstring>
#include <string>

using namespace chip::app::Clusters;

constexpr const char * LOG_TAG = "matter_device_translator";

namespace unify::matter_bridge {

/**
 * The API provides possible matched matter device types from list of clusters.
 */
std::vector<chip::DeviceTypeId>
device_translator::get_device_types(const std::vector<EmberAfCluster> & translated_matter_clusters) const
{
    std::vector<chip::DeviceTypeId> possible_device_list = select_possible_device_types_on_score(translated_matter_clusters, *this);
    if (!possible_device_list.empty())
    {
        sl_log_info(LOG_TAG, "Prioritized Matter Device Type:'%s'",
                    matter_device_type_vs_clusters_map.find(possible_device_list[0])->second.device_type_name);
        return possible_device_list;
    }
    else
    {
        sl_log_warning(LOG_TAG, "No matching Matter Device Type found");
    }

    return possible_device_list;
}

std::optional<const char *> device_translator::get_device_name(chip::DeviceTypeId device_id) const
{
    const auto it = matter_device_type_vs_clusters_map.find(device_id);
    if (it != matter_device_type_vs_clusters_map.end())
    {
        return it->second.device_type_name;
    }
    else
    {
        return std::nullopt;
    }
}

std::optional<chip::AttributeId> device_translator::get_attribute_id(const std::string & cluster_name,
                                                                     const std::string & attribute_name) const
{
    const auto attribute_map = unify_to_matter_attribute_id_map.find(cluster_name);
    if (attribute_map != unify_to_matter_attribute_id_map.end())
    {
        auto attribute_id = attribute_map->second.find(attribute_name);
        if (attribute_id != attribute_map->second.end())
        {
            return attribute_id->second;
        }
    }
    return std::nullopt;
}

std::optional<chip::CommandId> device_translator::get_command_id(const std::string & cluster_name,
                                                                 const std::string & command_name) const
{
    const auto command_map = unify_to_matter_command_id_map.find(cluster_name);
    if (command_map != unify_to_matter_command_id_map.end())
    {
        const auto command_id = command_map->second.find(command_name);
        if (command_id != command_map->second.end())
        {
            return command_id->second;
        }
    }
    return std::nullopt;
}

std::optional<chip::ClusterId> device_translator::get_cluster_id(const std::string & cluster_name) const
{
    const auto cluster_map_iter = unify_cluster_id_map.find(cluster_name);
    if (cluster_map_iter != unify_cluster_id_map.end())
    {
        return cluster_map_iter->second;
    }
    return std::nullopt;
}

std::optional<chip::ClusterId> device_translator::get_matter_cluster_id(const std::string & matter_cluster_name) const
{
    const auto cluster_map_iter = matter_cluster_id_map.find(matter_cluster_name);
    if (cluster_map_iter != matter_cluster_id_map.end())
    {
        return cluster_map_iter->second;
    }
    return std::nullopt;
}

std::optional<chip::CommandId> device_translator::get_matter_command_id(const std::string & cluster_name,
                                                                        const std::string & command_name) const
{
    const auto command_map = matter_command_id_map.find(cluster_name);
    if (command_map != matter_command_id_map.end())
    {
        const auto command_id = command_map->second.find(command_name);
        if (command_id != command_map->second.end())
        {
            return command_id->second;
        }
    }
    return std::nullopt;
}

std::optional<chip::AttributeId> device_translator::get_matter_attribute_id(const std::string & cluster_name,
                                                                            const std::string & attribute_name) const
{
    const auto attribute_map = matter_attribute_id_map.find(cluster_name);
    if (attribute_map != matter_attribute_id_map.end())
    {
        auto attribute_id = attribute_map->second.find(attribute_name);
        if (attribute_id != attribute_map->second.end())
        {
            return attribute_id->second;
        }
    }
    return std::nullopt;
}

} // namespace unify::matter_bridge
