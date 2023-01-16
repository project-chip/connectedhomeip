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
 * @brief Unify matter device mapper
 *
 * The purpose of the device mapper is to translate unify terms into matter terms.
 * The device mapper is capable of doing the following:
 * - Given a list of unify cluster names find the best matching device type
 * - Given a unify cluster name return the matter cluster id
 * - Given a unify attribute name return the matter attribute id
 * - Given a unify Command name return the matter command id
 *
 * @{
 */

#ifndef MATTER_DEVICE_TRANSLATOR_HPP
#define MATTER_DEVICE_TRANSLATOR_HPP

// Matter library
#include "matter.h"
#include <app-common/zap-generated/ids/Clusters.h>

// Unify library
#include "unify_node_state_monitor.hpp"

// Standard library
#include <optional>
#include <string>
#include <vector>

namespace unify::matter_bridge {
/**
 * @brief Device translator class.
 */
class device_translator
{
public:
    /**
     * @brief Given list of clustername find possible matching device types.
     *
     * @param clusters Vector of clusters to find device type for.
     * @return vector of possible device types sorted by most likely, or an empty
     * vector if none is found.
     */
    virtual std::vector<chip::DeviceTypeId> get_device_types(const std::vector<EmberAfCluster> & translated_matter_clusters) const;

    /**
     * @brief It provides the matter device name from device id.
     *
     * @param device_id the device id.
     * @return std::optional containing the device name or std::nullopt_t.
     */
    virtual std::optional<const char *> get_device_name(chip::DeviceTypeId device_id) const;

    /**
     * @brief Get the cluster id given a unify cluster name.
     *
     * @param cluster_name
     * @return std::optional containing the cluster id or std::nullopt_t.
     */
    virtual std::optional<chip::ClusterId> get_cluster_id(const std::string & cluster_name) const;

    /**
     * @brief Get the attribute id given a unify cluster name and attribute name.
     *
     * @param cluster_name
     * @param attribute_name
     * @return std::optional containg the attribute id or std::nullopt_t.
     */

    virtual std::optional<chip::AttributeId> get_attribute_id(const std::string & cluster_name,
                                                              const std::string & attribute_name) const;

    /**
     * @brief Get the command id given a unify cluster name and command name.
     *
     * @param cluster_name
     * @param attribute_name
     * @return std::optional containing the command id or std::nullopt_t.
     */

    virtual std::optional<chip::CommandId> get_command_id(const std::string & cluster_name, const std::string & command_name) const;

    /**
     * @brief Get the cluster id given a matter cluster name.
     *
     * @param matter_cluster_name
     * @return std::optional containing the cluster id or std::nullopt_t.
     */
    virtual std::optional<chip::ClusterId> get_matter_cluster_id(const std::string & matter_cluster_name) const;

    /**
     * @brief Get the command id given a matter cluster name and command name.
     *
     * @param matter_cluster_name
     * @param matter_command_name
     * @return std::optional containg the attribute id or std::nullopt_t.
     */
    virtual std::optional<chip::CommandId> get_matter_command_id(const std::string & matter_cluster_name,
                                                                 const std::string & matter_command_name) const;

    /**
     * @brief Get the attribute id given a matter cluster name and attribute name.
     *
     * @param matter_cluster_name
     * @param matter_attribute_name
     * @return std::optional containing the attribute id or std::nullopt_t.
     */
    virtual std::optional<chip::AttributeId> get_matter_attribute_id(const std::string & cluster_name,
                                                                     const std::string & attribute_name) const;

    virtual ~device_translator() = default;

    static const device_translator & instance()
    {
        static device_translator me;
        return me;
    }
};
} // namespace unify::matter_bridge

#endif // MATTER_DEVICE_TRANSLATOR_HPP
/** @} end matter_device_mapper */
