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

static unify::matter_bridge::device_translator dev_translator;

static uint8_t compare_clusters(chip::ClusterId cluster_id, const std::vector<DeviceClusterData> & matter_device_clusters, bool only_count_required_clusters)
{
    bool it_was_required_cluster_match = false;
    const auto it =
        std::find_if(matter_device_clusters.begin(), matter_device_clusters.end(), [cluster_id, &it_was_required_cluster_match](const DeviceClusterData & cluster) {
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
        } else if (!only_count_required_clusters) {
            return 1;
        }
    }
    return 0;
}

static uint8_t score_comparisons_of_cluster_lists(const std::vector<EmberAfCluster> & matter_clusters,
                                                  const std::vector<DeviceClusterData> & matter_device_clusters, bool only_count_required_clusters)
{
    uint8_t count = 0;
    for (const auto & matter_cluster : matter_clusters)
    {
        count += compare_clusters(matter_cluster.clusterId, matter_device_clusters, only_count_required_clusters);
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

static bool compare_attributes(const EmberAfAttributeMetadata * cluster_attributes,
                        const std::vector<std::string> & required_cluster_attributes)
{
    // TODO: Make a comparison of the cluster attributes with the required attributes
    return true;
}

static bool compare_commands(const chip::CommandId * commands, const std::vector<std::string> & required_commands)
{
    // TODO: Make a comparison of the cluster commands with the required commands
    return true;
}

static bool matter_clusters_conform_to_device_type(const std::vector<EmberAfCluster> & matter_cluster_list,
                                            const std::vector<DeviceClusterData> & device_type_cluster_data,
                                            bool conform_to_spec_for_clusters, bool conform_to_spec_for_attributes,
                                            bool conform_to_spec_for_commands)
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

            if (conform_to_spec_for_clusters)
            {
                const int8_t matter_cluster_index =
                    check_if_cluster_in_array_of_clusters(device_cluster_type.value(), matter_cluster_list);
                if (matter_cluster_index == -1)
                {
                    return false;
                }

                // Check mandatory attributes and commands for each cluster
                if (conform_to_spec_for_attributes)
                {
                    if (!compare_attributes(matter_cluster_list.at(matter_cluster_index).attributes,
                                            matter_device_cluster_data.required_attributes))
                    {
                        return false;
                    }
                }

                if (conform_to_spec_for_commands)
                {
                    if (!compare_commands(matter_cluster_list.at(matter_cluster_index).acceptedCommandList,
                                          matter_device_cluster_data.required_commands))
                    {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

std::vector<cluster_score> compute_device_type_match_score(const std::vector<EmberAfCluster> & matter_cluster_list)
{

    // contains scores that reflect if all list clusters present under a given
    // device type
    std::vector<cluster_score> device_type_match_score;
    sl_log_debug(LOG_TAG, "Computing device type match score for %d clusters", matter_cluster_list.size());

    for (const auto & [matter_device_type_id, matter_device] : matter_device_type_vs_clusters_map)
    {
        bool device_conformance = matter_clusters_conform_to_device_type(
            matter_cluster_list, matter_device.clusters, matter_device.conform_clusters_to_spec,
            matter_device.conform_attributes_to_spec, matter_device.conform_commands_to_spec);
        if (!device_conformance)
        {
            continue;
        }
        uint8_t clusters_matched = score_comparisons_of_cluster_lists(matter_cluster_list, matter_device.clusters, false);
        uint8_t required_clusters_matched = score_comparisons_of_cluster_lists(matter_cluster_list, matter_device.clusters, true);

        unsigned int addtional_matter_clusters = matter_cluster_list.size() - clusters_matched;
        unsigned int matter_device_miss_count  = matter_device.clusters.size() - clusters_matched;
        device_type_match_score.emplace_back(addtional_matter_clusters, matter_device_miss_count, required_clusters_matched, matter_device_type_id);
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

std::vector<chip::DeviceTypeId> select_possible_device_types_on_score(std::vector<EmberAfCluster> matter_clusters_to_match)
{
    std::vector<cluster_score> device_type_match_score = compute_device_type_match_score(matter_clusters_to_match);

    sl_log_debug(LOG_TAG, "Device cluster score length %d", device_type_match_score.size());
    sl_log_debug(LOG_TAG, "Selecting device types based on score");
    // Select possible device types based on score list.
    std::vector<chip::DeviceTypeId> possible_device_list;
    auto best_score = device_type_match_score.begin();
    const uint8_t skew_score_computation = 20;
    if (best_score != device_type_match_score.end())
    {
        sl_log_debug(LOG_TAG, "Adding device type %d to possible device list", best_score->device_type_id);
        possible_device_list.push_back(best_score->device_type_id);
        auto iter = best_score;
        ++iter;
        for (; iter != device_type_match_score.end(); ++iter)
        {
            uint8_t best_numerical_score = skew_score_computation + best_score->matter_miss_count + best_score->extra_matter_clusters_count - best_score->required_matter_clusters_count;
            uint8_t iter_numerical_score = skew_score_computation + iter->matter_miss_count + iter->extra_matter_clusters_count - iter->required_matter_clusters_count;
            if (best_numerical_score >= iter_numerical_score)
            {
                best_score = iter;
                possible_device_list.insert(possible_device_list.begin(), iter->device_type_id);
            }
        }
    }
    return possible_device_list;
}