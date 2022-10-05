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
 * @defgroup matter_cluster_translator
 * @brief TODO: Write brief for matter_cluster_translator
 *
 * TODO: Write component description for matter_cluster_translator
 *
 * @{
 */

#ifndef MATTER_CLUSTER_TRANSLATOR_HPP
#define MATTER_CLUSTER_TRANSLATOR_HPP

#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>

namespace unify::matter_bridge {
/**
 * The purpose of the cluster translator is to handle read/write attribute
 * calls as well as matter commands. Since the matter data model and the UIC
 * data model are very similar we will generally be able to do a 1-1
 * translation.
 *
 * The translator has a ZAP generated handler, which implements a generic
 * handler for all clusters which is able to send translate Matter commands
 * into mqtt messages. The translator uses
 * `InteractionModelEngine::RegisterCommandHandler` to register itself with
 * the matter application framework. As the default behavior the translator
 * will directly translate the matter command into a unify mqtt command.
 *
 * The translator will also handle attribute read and attribute writes. For
 * attributes the translator uses the system
 * `registerAttributeAccessOverride, when an attribute read is requested the
 * Unify Reported value should be reported when an attribute write is
 * requested the corresponding /WriteAttribute command is published on the
 * mqtt side.
 *
 * The Unify Cluster translator is not required to check the capabilities of
 * a node before sending WriteAttributes or other commands.
 *
 */
class matter_cluster_translator
{

    /**
     * @brief Register access an interface to a cluster
     *
     * Register a cluster translator.
     *
     * @param command_handler
     * @param attribute_access
     * @return true on success
     */
    bool register_cluster(chip::ClusterId, const CommandHandlerInterface & command_handler,
                          const AttributeAccessInterface & attribute_access);
}

} // namespace unify::matter_bridge
#endif // MATTER_CLUSTER_TRANSLATOR_HPP
/** @} end matter_cluster_translator */
