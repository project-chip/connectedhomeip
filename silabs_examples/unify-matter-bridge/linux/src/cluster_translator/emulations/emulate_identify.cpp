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

#include "emulate_identify.hpp"

// UIC
#include "sl_log.h"

// Matter
#include <app-common/zap-generated/cluster-enums.h>

// Standard library
#include <map>

namespace unify::matter_bridge {

using namespace chip::app;
using namespace chip::app::Clusters;

#define LOG_TAG "EmulateIdentify"

CHIP_ERROR EmulateIdentify::emulate(
    const node_state_monitor::cluster & unify_cluster, matter_cluster_builder & cluster_builder,
    std::map<chip::ClusterId, std::map<chip::AttributeId, EmulatorInterface *>> & emulation_attribute_handler_map,
    std::map<chip::ClusterId, std::map<chip::CommandId, EmulatorInterface *>> & emulation_command_handler)
{
    // Identify cluster emulation handler registered for IdentifyType attribute
    sl_log_debug(LOG_TAG, "Emulating IdentifyType attribute for Identify cluster");
    emulation_attribute_handler_map[Identify::Id][Identify::Attributes::IdentifyType::Id] = this;

    // Add IdentifyType attribute to the matter cluster
    cluster_builder.attributes.push_back(EmberAfAttributeMetadata{ Identify::Attributes::IdentifyType::Id, ZCL_ENUM8_ATTRIBUTE_ID,
                                                                   1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() });

    return CHIP_NO_ERROR;
}

CHIP_ERROR EmulateIdentify::read_attribute(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    sl_log_debug(LOG_TAG, "Read IdentifyType attribute");

    // IdentifyType is a mandatory attribute we default it to None
    if (aPath.mAttributeId == Identify::Attributes::IdentifyType::Id)
    {
        return aEncoder.Encode(Identify::IdentifyIdentifyType::EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_NONE);
    }

    sl_log_warning(LOG_TAG, "Emulation identify activated for unknown attribute id %d under cluster id %d", aPath.mAttributeId,
                   aPath.mClusterId);
    return CHIP_ERROR_INVALID_ARGUMENT;
}

} // namespace unify::matter_bridge