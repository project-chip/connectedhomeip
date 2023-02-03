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

#ifndef EMULATOR_HPP
#define EMULATOR_HPP

#include "matter.h"
#include "matter_endpoint_builder.hpp"
#include "unify_node_state_monitor.hpp"

namespace unify::matter_bridge {

using namespace chip::app;
using namespace chip::app::Clusters;

class EmulatorInterface
{
public:
    virtual ~EmulatorInterface() = default;

    /**
     * @brief Matter clusterId for which this emualtor applies
     *
     * @return chip::ClusterId
     */
    virtual chip::ClusterId emulated_cluster() const = 0;

    /**
     * @brief Unify Cluster name for this emulator
     *
     * @return const char*
     */
    virtual const char * emulated_cluster_name() const = 0;

    /**
     * @brief List of attributes which this emulator handles
     *
     * @return std::vector<chip::AttributeId>
     */
    virtual std::vector<chip::AttributeId> emulated_attributes() const { return {}; }

    /**
     * @brief List of commands which this emulator handles
     *
     * @return std::vector<chip::CommandId>
     */
    virtual std::vector<chip::CommandId> emulated_commands() const { return {}; }

    /**
     * @brief Emulate attributes and commands for a cluster
     *
     * @param unify_cluster
     * @param cluster_builder Matter cluster which contains already mapped attributes and commands
     * @return CHIP_ERROR
     */
    virtual CHIP_ERROR emulate(const node_state_monitor::cluster & unify_cluster, matter_cluster_builder & cluster_builder)
    {
        return CHIP_NO_ERROR;
    }

    /**
     * @brief Handle a clusters emulated command
     *
     * @param handlerContext
     * @return CHIP_ERROR
     */
    virtual CHIP_ERROR command(CommandHandlerInterface::HandlerContext & handlerContext) { return CHIP_ERROR_NOT_IMPLEMENTED; }

    /**
     * @brief Handle a clusters emulated attribute read
     *
     * @param aPath
     * @param aEncoder
     * @return CHIP_ERROR
     */
    virtual CHIP_ERROR read_attribute(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    /**
     * @brief Handle a clusters emulated attribute write
     *
     * @param aPath
     * @param aDecoder
     * @return CHIP_ERROR
     */
    virtual CHIP_ERROR write_attribute(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
};

} // namespace unify::matter_bridge

#endif // EMULATOR_HPP
