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
#include "matter_device_type_selection.hpp"

// Matter library
#include <app-common/zap-generated/ids/Clusters.h>

// Application library
#include "matter_device_translator.hpp"
#include "matter_device_types_clusters_list.inc"

// Unify components
#include "sl_log.h"

// Standard library
#include <algorithm>
#include <cstring>
#include <unordered_set>

#define LOG_TAG "matter_device_type_selection"

static uint8_t compare_clusters(chip::ClusterId cluster_id, const std::vector<DeviceClusterData> & matter_device_clusters,
                                bool only_count_required_clusters, const unify::matter_bridge::device_translator & dev_translator)
{
    bool it_was_required_cluster_match = false;
    const auto it                      = std::find_if(matter_device_clusters.begin(), matter_device_clusters.end(),
                                 [cluster_id, &it_was_required_cluster_match, dev_translator](const DeviceClusterData & cluster) {
                                     auto device_cluster_id = dev_translator.get_matter_cluster_id(cluster.cluster_name);
                                     if (!device_cluster_id.has_value())
                                     {
                                         return false;
                                     }
                                     if (cluster_id == device_cluster_id.value())
                                     {
                                         it_was_required_cluster_match = cluster.is_mandatory;
                                         return true;
                                     }

                                     return false;
                                 });
    if (it != matter_device_clusters.end())
    {
        if (only_count_required_clusters && it_was_required_cluster_match)
        {
            return 1;
        }
        else if (!only_count_required_clusters)
        {
            return 1;
        }
    }
    return 0;
}

static uint8_t score_comparisons_of_cluster_lists(const std::vector<EmberAfCluster> & matter_clusters,
                                                  const std::vector<DeviceClusterData> & matter_device_clusters,
                                                  bool only_count_required_clusters,
                                                  const unify::matter_bridge::device_translator & dev_translator)
{
    uint8_t count = 0;
    for (const auto & matter_cluster : matter_clusters)
    {
        count += compare_clusters(matter_cluster.clusterId, matter_device_clusters, only_count_required_clusters, dev_translator);
    }
    return count;
}

static int8_t check_if_cluster_in_array_of_clusters(const chip::ClusterId matter_cluster,
                                                    const std::vector<EmberAfCluster> & matter_clusters)
{
    const auto it = std::find_if(matter_clusters.begin(), matter_clusters.end(),
                                 [matter_cluster](EmberAfCluster cluster) { return matter_cluster == cluster.clusterId; });
    if (it != matter_clusters.end())
    {
        return it - matter_clusters.begin();
    }
    return -1;
}

bool compare_attributes(const EmberAfAttributeMetadata * attributes, const uint16_t attribute_count,
                        const std::string required_cluster_name, const std::vector<std::string> & required_attributes,
                        const unify::matter_bridge::device_translator & dev_translator)
{
    for (const auto & required_attribute : required_attributes)
    {
        bool required_attribute_found = false;
        auto attribute_id             = dev_translator.get_matter_attribute_id(required_cluster_name, required_attribute);

        if (!attribute_id.has_value())
        {
            return false;
        }

        for (uint8_t i = 0; i < attribute_count; i++)
        {
            if (attributes[i].attributeId == attribute_id.value())
            {
                required_attribute_found = true;
                break;
            }
        }

        if (!required_attribute_found)
        {
            sl_log_debug(LOG_TAG, "Attribute %s not found in cluster %s", required_attribute.c_str(),
                         required_cluster_name.c_str());
            return false;
        }
    }
    return true;
}

bool compare_commands(const chip::CommandId * commands, const std::string required_cluster_name,
                      const std::vector<std::string> & required_commands,
                      const unify::matter_bridge::device_translator & dev_translator)
{

    uint8_t commands_count = sizeof(commands);
    uint8_t end_of_vector  = -1;
    for (const auto & required_command : required_commands)
    {
        bool required_command_found = false;
        auto command_id             = dev_translator.get_matter_command_id(required_cluster_name, required_command);
        if (!command_id.has_value())
        {
            return false;
        }

        for (uint8_t i = 0; i < commands_count; i++)
        {
            if (commands[i] == end_of_vector)
            {
                // End of array
                break;
            }
            if (commands[i] == command_id.value())
            {
                required_command_found = true;
                break;
            }
        }
        if (!required_command_found)
        {
            sl_log_debug(LOG_TAG, "Command %s not found in cluster %s", required_command.c_str(), required_cluster_name.c_str());
            return false;
        }
    }
    return true;
}

bool matter_clusters_conform_to_device_type(const std::vector<EmberAfCluster> & matter_cluster_list,
                                            const std::vector<DeviceClusterData> & device_type_cluster_data,
                                            const unify::matter_bridge::device_translator & dev_translator)
{
    for (const auto & matter_device_cluster_data : device_type_cluster_data)
    {
        if (matter_device_cluster_data.is_mandatory)
        {
            auto device_cluster_type = dev_translator.get_matter_cluster_id(matter_device_cluster_data.cluster_name);
            if (!device_cluster_type.has_value())
            {
                sl_log_warning(LOG_TAG, "Failed to get cluster id for cluster %s. Stating device does not conform to spec.",
                               matter_device_cluster_data.cluster_name);
                return false;
            }

            // Agreed to not count scenes as a spec compliant cluster as Matter spec is not sure on how to implement it
            if (dev_translator.m_spec_compliant && strcmp(matter_device_cluster_data.cluster_name, "Scenes") != 0)
            {
                const int8_t matter_cluster_index =
                    check_if_cluster_in_array_of_clusters(device_cluster_type.value(), matter_cluster_list);
                if (matter_cluster_index == -1)
                {
                    return false;
                }

                if (!compare_attributes(matter_cluster_list.at(matter_cluster_index).attributes,
                                        matter_cluster_list.at(matter_cluster_index).attributeCount,
                                        matter_device_cluster_data.cluster_name, matter_device_cluster_data.required_attributes,
                                        dev_translator))
                {
                    return false;
                }

                if (!compare_commands(matter_cluster_list.at(matter_cluster_index).acceptedCommandList,
                                      matter_device_cluster_data.cluster_name, matter_device_cluster_data.required_commands,
                                      dev_translator))
                {
                    return false;
                }
            }
        }
    }
    return true;
}

std::vector<cluster_score> compute_device_type_match_score(const std::vector<EmberAfCluster> & matter_cluster_list,
                                                           const unify::matter_bridge::device_translator & dev_translator)
{

    // contains scores that reflect if all list clusters present under a given
    // device type
    std::vector<cluster_score> device_type_match_score;
    sl_log_debug(LOG_TAG, "Computing device type match score for %d clusters", matter_cluster_list.size());

    for (const auto & [matter_device_type_id, matter_device] : matter_device_type_vs_clusters_map)
    {
        bool device_conformance =
            matter_clusters_conform_to_device_type(matter_cluster_list, matter_device.clusters, dev_translator);
        if (!device_conformance)
        {
            continue;
        }
        uint8_t clusters_matched =
            score_comparisons_of_cluster_lists(matter_cluster_list, matter_device.clusters, false, dev_translator);
        uint8_t required_clusters_matched =
            score_comparisons_of_cluster_lists(matter_cluster_list, matter_device.clusters, true, dev_translator);

        unsigned int addtional_matter_clusters = matter_cluster_list.size() - clusters_matched;
        unsigned int matter_device_miss_count  = matter_device.clusters.size() - clusters_matched;
        device_type_match_score.emplace_back(addtional_matter_clusters, matter_device_miss_count, required_clusters_matched,
                                             matter_device_type_id);
    }
    std::sort(device_type_match_score.begin(), device_type_match_score.end(), [](const cluster_score & a, const cluster_score & b) {
        // Sorting algorithm first sort based on matter_miss_count, if that
        // match sort based on matter_miss_count.
        if (a.matter_miss_count < b.matter_miss_count)
        {
            return true;
        }
        else if (b.matter_miss_count < a.matter_miss_count)
        {
            return false;
        }
        else
        {
            // There is an equal amount of matter miss clusters, now we sort based on matched required clusters count.
            if (a.required_matter_clusters_count > b.required_matter_clusters_count)
            {
                return true;
            }
            else if (b.required_matter_clusters_count > a.required_matter_clusters_count)
            {
                return false;
            }

            // extra_matter_cluster_count match on a and b, now we sort based on
            // matter_miss_count.
            return a.extra_matter_clusters_count <= b.extra_matter_clusters_count;
        }
    });

    return device_type_match_score;
}

std::vector<chip::DeviceTypeId>
select_possible_device_types_on_score(std::vector<EmberAfCluster> matter_clusters_to_match,
                                      const unify::matter_bridge::device_translator & dev_translator)
{
    std::vector<cluster_score> device_type_match_score = compute_device_type_match_score(matter_clusters_to_match, dev_translator);

    sl_log_debug(LOG_TAG, "Device cluster score length %d", device_type_match_score.size());
    sl_log_debug(LOG_TAG, "Selecting device types based on score");
    // Select possible device types based on score list.
    std::vector<chip::DeviceTypeId> possible_device_list;
    auto best_score                      = device_type_match_score.begin();
    const uint8_t skew_score_computation = 20;
    if (best_score != device_type_match_score.end())
    {
        sl_log_debug(LOG_TAG, "Adding device type %d to possible device list", best_score->device_type_id);
        possible_device_list.push_back(best_score->device_type_id);
        auto iter = best_score;
        ++iter;
        for (; iter != device_type_match_score.end(); ++iter)
        {
            uint8_t best_numerical_score = skew_score_computation + best_score->matter_miss_count +
                best_score->extra_matter_clusters_count - best_score->required_matter_clusters_count;
            uint8_t iter_numerical_score = skew_score_computation + iter->matter_miss_count + iter->extra_matter_clusters_count -
                iter->required_matter_clusters_count;
            if (best_numerical_score >= iter_numerical_score)
            {
                best_score = iter;
                possible_device_list.insert(possible_device_list.begin(), iter->device_type_id);
            }
        }
    }
    return possible_device_list;
}
