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

/**
 * @defgroup matter_device_mapper
 * @brief Unify matter device mapper selection algorithms
 *
 * This module exposes the selection algorithms for the matter device mapper.
 *
 * @{
 */

#ifndef MATTER_DEVICE_TYPE_SELECTION_HPP
#define MATTER_DEVICE_TYPE_SELECTION_HPP

// Unify components
#include "unify_node_state_monitor.hpp"

// Matter components
#include <app-common/zap-generated/ids/Clusters.h>
#include "matter.h"

// Standard library
#include <string>
#include <vector>

/**
 * @brief Cluster Score struct
 *
 * Holds the information on cluster scores used to determine possible device
 * types in \ref get_device_types.
 *
 */
struct cluster_score
{
    /// Number of matter clusters that didn't match matter device clusters (0 means all
    /// match).
    unsigned int extra_matter_clusters_count;
    /// Number of matter clusters that didn't match the device clusters (0 means
    /// all match perfect fit).
    unsigned int matter_miss_count;
    /// Number of required matter clusters matches
    unsigned int required_matter_clusters_count; 
    chip::DeviceTypeId device_type_id;
    cluster_score(unsigned int _extra_matter_clusters_count, unsigned int _matter_miss_count, unsigned int _required_matter_clusters_count, chip::DeviceTypeId _device_type_id) :
        extra_matter_clusters_count(_extra_matter_clusters_count), matter_miss_count(_matter_miss_count), required_matter_clusters_count(_required_matter_clusters_count), device_type_id(_device_type_id)
    {}
};


/**
 * @brief Select possible device types based of matchs core.
 *
 * This function returns a list of possible device types based on the cluster
 * scores.
 *
 * @param device_type_match_score The cluster list
 * @return A list of possible device types
 */
std::vector<chip::DeviceTypeId> select_possible_device_types_on_score(std::vector<EmberAfCluster> translated_matter_clusters);

#endif // MATTER_DEVICE_TYPE_SELECTION_HPP