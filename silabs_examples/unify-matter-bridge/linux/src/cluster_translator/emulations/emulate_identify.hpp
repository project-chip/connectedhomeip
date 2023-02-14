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
    const char * emulated_cluster_name() const override { return "Identify"; }

    chip::ClusterId emulated_cluster() const override { return Identify::Id; };

    std::vector<chip::AttributeId> emulated_attributes() const override { return { Identify::Attributes::IdentifyType::Id }; }

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
    emulate(const node_state_monitor::cluster & unify_cluster, matter_cluster_builder & cluster_builder) override
    {
        // Add IdentifyType attribute to the matter cluster
        cluster_builder.attributes.emplace_back(EmberAfAttributeMetadata{ZAP_EMPTY_DEFAULT(),
                                                                         Identify::Attributes::IdentifyType::Id,
                                                                         1,
                                                                         ZAP_TYPE(ENUM8),
                                                                         ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE)});

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR read_attribute(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override
    {
        // IdentifyType is a mandatory attribute we default it to None
        if (aPath.mAttributeId == Identify::Attributes::IdentifyType::Id)
        {
            return aEncoder.Encode(Identify::IdentifyIdentifyType::EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_NONE);
        }

        return CHIP_ERROR_INVALID_ARGUMENT;
    };
};

} // namespace unify::matter_bridge

#endif // EMULATE_IDENTIFY_HPP
