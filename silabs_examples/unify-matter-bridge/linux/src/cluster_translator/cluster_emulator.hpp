
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

#ifndef CLUSTER_EMULATOR_HPP
#define CLUSTER_EMULATOR_HPP

#include "matter_endpoint_builder.hpp"
#include "unify_node_state_monitor.hpp"

// Emulation of commands and attributes
#include "emulator.hpp"

namespace unify::matter_bridge {

/**
 * @brief Implement emulation of Attribute and Commands for clusters.
 *
 * The role of the cluster emulator is to emulate missing commands and cluster attributes, which
 * are not defined by the UCL specification.
 */
class ClusterEmulator
{
public:
    ClusterEmulator();

    /**
     * @brief Add command and attributes to the clusters on the endpoint
     *
     * This function will all the commands and clusters to endpoint descriptor
     * which the endpoint_builder holds. Which commands an attributes are
     * emulated can be checked with  is_command_emulated and is_attribute_emulated
     */
    void add_emulated_commands_and_attributes(const node_state_monitor::cluster & unify_clusters, matter_cluster_builder &);

    /**
     * @brief Check if a command is emulated
     *
     * @return true
     * @return false
     */
    bool is_command_emulated(const chip::app::ConcreteCommandPath &) const;

    /**
     * @brief Check if an attribute is emulated
     *
     * @return true
     * @return false
     */
    bool is_attribute_emulated(const chip::app::ConcreteAttributePath &) const;

    /**
     * @brief Execute an emulated command
     *
     * @param handlerContext
     * @return
     *  - CHIP_ERROR_OK if the command was successfully emulated.
     *  - CHIP_ERROR_NOT_IMPLEMENTED If the command is not emulated
     *
     */
    CHIP_ERROR invoke_command(chip::app::CommandHandlerInterface::HandlerContext & handlerContext) const;

    /**
     * @brief Read an emulated attribute
     *
     * @param aPath
     * @param aEncoder
     *  - CHIP_ERROR_OK read was successfully emulated.
     *  - CHIP_ERROR_READ_FAILED if the value of the attribute is not known.
     *  - CHIP_ERROR_NOT_IMPLEMENTED If the attribute is not emulated
     */
    CHIP_ERROR read_attribute(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder);

    /**
     * @brief Write an emulated attribute
     *
     * @param aPath
     * @param aDecoder
     * @return
     *  - CHIP_ERROR_OK write was successfully emulated.
     *  - CHIP_ERROR_WRITE_FAILED the attribute is not writable.
     *  - CHIP_ERROR_NOT_IMPLEMENTED If the attribute is not emulated
     */
    CHIP_ERROR write_attribute(const chip::app::ConcreteDataAttributePath & aPath,
                               chip::app::AttributeValueDecoder & aDecoder) const;

private:
    // Emulation functions
    uint32_t read_feature_map_revision(const ConcreteReadAttributePath & aPath) const;
    uint32_t read_cluster_revision(const ConcreteReadAttributePath & aPath) const;

    std::map<std::pair<chip::ClusterId, chip::AttributeId>, std::shared_ptr<EmulatorInterface>> cluster_emulators_attribute_id_map;
    std::map<std::pair<chip::ClusterId, chip::CommandId>, std::shared_ptr<EmulatorInterface>> cluster_emulators_command_id_map;
    std::map<std::string, std::shared_ptr<EmulatorInterface>> cluster_emulators_string_map;
};

} // namespace unify::matter_bridge
#endif
