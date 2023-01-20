/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#ifndef EMULATE_IDENTIFY_HPP
#define EMULATE_IDENTIFY_HPP

// Emulator interface
#include "emulator.hpp"

namespace unify::matter_bridge {

using namespace chip::app;
using namespace chip::app::Clusters;

class EmulateIdentify : public EmulatorInterface
{
public:
    /**
     * @brief Emulate Identify cluster this will emulate defined commands and
     * attributes
     *
     * @param unify_cluster
     * @param cluster_builder
     * @param emulation_handler_map
     * @return CHIP_ERROR
     */
    CHIP_ERROR
    emulate(const node_state_monitor::cluster & unify_cluster, matter_cluster_builder & cluster_builder,
            std::map<chip::ClusterId, std::map<chip::AttributeId, EmulatorInterface *>> & emulation_attribute_handler_map,
            std::map<chip::ClusterId, std::map<chip::CommandId, EmulatorInterface *>> & emulation_command_handler) override;

    /**
     * @brief Read the emulated IdentifyType attribute
     *
     * @param aPath
     * @param aEncoder
     * @return CHIP_ERROR
     */
    CHIP_ERROR read_attribute(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
};

} // namespace unify::matter_bridge

#endif // EMULATE_IDENTIFY_HPP