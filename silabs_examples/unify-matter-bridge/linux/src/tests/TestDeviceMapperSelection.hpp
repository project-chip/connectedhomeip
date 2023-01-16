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

// Device selection header
#include "matter_device_type_selection.hpp"
#include "matter_device_types_clusters_list.inc"

// Exposing extra functions of device selection for testing
std::vector<cluster_score> compute_device_type_match_score(const std::vector<EmberAfCluster> & matter_cluster_list);
bool compare_commands(const chip::CommandId * commands, const std::string required_cluster_name,
                      const std::vector<std::string> & required_commands);
bool compare_attributes(const EmberAfAttributeMetadata * attributes, const uint16_t attribute_count,
                        const std::string required_cluster_name, const std::vector<std::string> & required_attributes);
bool matter_clusters_conform_to_device_type(const std::vector<EmberAfCluster> & matter_cluster_list,
                                            const std::vector<DeviceClusterData> & device_type_cluster_data,
                                            bool conform_to_spec_for_clusters, bool conform_to_spec_for_attributes,
                                            bool conform_to_spec_for_commands);
