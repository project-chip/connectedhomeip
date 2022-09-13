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
#include "matter_device_translator.hpp"
#include "matter_device_types_clusters_list_updated.inc"
#include "matter_device_mapper.inc"

#include <unordered_map>
#include <string>
#include <algorithm>
#include <stdio.h>
#include <cstring>

using namespace chip::app::Clusters;



namespace unify::matter_bridge
{
static uint8_t device_type_mapper_compare_cluster_lists(
  const std::vector<const char *> &unify_clusters,
  const std::vector<const char *> &matter_clusters)
{
  uint8_t count = 0;
  for (const auto &unify_cluster: unify_clusters) {
    // Find matching unify_cluster
    const auto it
      = std::find_if(matter_clusters.begin(),
                     matter_clusters.end(),
                     [unify_cluster](const char *matter_cluster) {
                       return strcmp(unify_cluster, matter_cluster) == 0;
                     });
    if (it != matter_clusters.end()) {
      count = count + 1;
    }
  }
  return count;
}

/**
 * @brief Cluster Score struct
 *
 * Holds the information on cluster scores used to determine possible device
 * types in \ref get_device_types.
 *
 */
struct cluster_score {
  /// Number of unify clusters that didn't match matter clusters (0 means all
  /// match).
  unsigned int unify_miss_count;
  /// Number of matter clusters that didn't match the unify clusters (0 means
  /// all match).
  unsigned int matter_miss_count;
  chip::DeviceTypeId device_type_id;
  cluster_score(unsigned int unify_miss_count,
                unsigned int matter_miss_count,
                chip::DeviceTypeId device_type_id) :
    unify_miss_count(unify_miss_count),
    matter_miss_count(matter_miss_count),
    device_type_id(device_type_id)
  {}
};
/**
 * The API provides possible matched matter device types from list of clusters.
 */
std::vector<chip::DeviceTypeId> device_translator::get_device_types(
  const std::vector<const char *> &unify_cluster_list) const
{
  // contains scores that refelect if all list clusters present under a given
  // device type
  std::vector<cluster_score> device_type_match_score;

  for (const auto &[matter_device_type_id, matter_device]:
       matter_device_type_vs_clusters_map) {
    uint8_t clusters_matched
      = device_type_mapper_compare_cluster_lists(unify_cluster_list,
                                                 matter_device.first);

    unsigned int unify_miss_count
      = unify_cluster_list.size() - clusters_matched;
    unsigned int matter_miss_count
      = matter_device.first.size() - clusters_matched;
    device_type_match_score.emplace_back(unify_miss_count,
                                         matter_miss_count,
                                         matter_device_type_id);
  }
  std::sort(device_type_match_score.begin(),
            device_type_match_score.end(),
            [](const cluster_score &a, const cluster_score &b) {
              // Sorting algorithm first sort based on unify_miss_count, if that
              // match sort based on matter_miss_count.
              if (a.unify_miss_count < b.unify_miss_count) {
                return true;
              } else if (b.unify_miss_count < a.unify_miss_count) {
                return false;
              } else {
                // unify_miss_count match on a and b, now we sort based on
                // matter_miss_count.
                return a.matter_miss_count <= b.matter_miss_count;
              }
            });
  // Select possible device types based on score list.
  std::vector<chip::DeviceTypeId> possible_device_list;
  const auto it_first = device_type_match_score.begin();
  if (it_first != device_type_match_score.end()) {
    possible_device_list.push_back(it_first->device_type_id);
    auto iter = it_first;
    ++iter;
    for (; iter != device_type_match_score.end(); ++iter) {
      if ((it_first->unify_miss_count == iter->unify_miss_count)
          && (it_first->matter_miss_count == iter->matter_miss_count)) {
        possible_device_list.push_back(iter->device_type_id);
      }
    }
  }

  return possible_device_list;
}

std::optional<const char *>
  device_translator::get_device_name(chip::DeviceTypeId device_id) const
{
  const auto it = matter_device_type_vs_clusters_map.find(device_id);
  if (it != matter_device_type_vs_clusters_map.end()) {
    return it->second.second;
  } else {
    return std::nullopt;
  }
}

std::optional<chip::AttributeId>
  device_translator::get_attribute_id(const std::string &cluster_name,
                                      const std::string &attribute_name) const
{
  const auto attribute_map
    = unify_to_matter_attribute_id_map.find(cluster_name);
  if (attribute_map != unify_to_matter_attribute_id_map.end()) {
    auto attribute_id = attribute_map->second.find(attribute_name);
    if (attribute_id != attribute_map->second.end()) {
      return attribute_id->second;
    }
  }
  return std::nullopt;
}

std::optional<chip::CommandId>
  device_translator::get_command_id(const std::string &cluster_name,
                                    const std::string &command_name) const
{
  const auto command_map = unify_to_matter_command_id_map.find(cluster_name);
  if (command_map != unify_to_matter_command_id_map.end()) {
    const auto command_id = command_map->second.find(command_name);
    if (command_id != command_map->second.end()) {
      return command_id->second;
    }
  }
  return std::nullopt;
}

std::optional<chip::ClusterId>
  device_translator::get_cluster_id(const std::string &cluster_name) const
{
  const auto cluster_map_iter = unify_cluster_id_map.find(cluster_name);
  if (cluster_map_iter != unify_cluster_id_map.end()) {
    return cluster_map_iter->second;
  }
  return std::nullopt;
}

}  // namespace unify::matter_bridge
