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

#ifndef EMULATE_DOORLOCK_HPP
#define EMULATE_DOORLOCK_HPP

// Emulator interface
#include "emulator.hpp"

namespace unify::matter_bridge {

using namespace chip::app;
using namespace chip::app::Clusters;

class EmulateDoorLock : public EmulatorInterface
{
public:
    const char * emulated_cluster_name() const override { return "DoorLock"; }

    chip::ClusterId emulated_cluster() const override { return DoorLock::Id; };

    std::vector<chip::AttributeId> emulated_attributes() const override
    {
        return { DoorLock::Attributes::OperatingMode::Id, DoorLock::Attributes::SupportedOperatingModes::Id };
    }

    /**
     * @brief Emulate Doorlock cluster to emulate commands and attributes
     *
     * @param unify_cluster
     * @param cluster_builder
     * @return CHIP_ERROR
     */
    CHIP_ERROR
    emulate(const node_state_monitor::cluster & unify_cluster, matter_cluster_builder & cluster_builder) override
    {
        // Add OperatingMode and SupportedOperatingModes attribute to the matter cluster
        cluster_builder.attributes.emplace_back(EmberAfAttributeMetadata{ZAP_EMPTY_DEFAULT(),
                                                                         DoorLock::Attributes::OperatingMode::Id,
                                                                         1,
                                                                         ZAP_TYPE(ENUM8),
                                                                         ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE)});
        cluster_builder.attributes.emplace_back(EmberAfAttributeMetadata{ZAP_EMPTY_DEFAULT(),
                                                                         DoorLock::Attributes::SupportedOperatingModes::Id,
                                                                         2,
                                                                         ZAP_TYPE(BITMAP16),
                                                                         ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE)});

        return CHIP_NO_ERROR;
    }

    /**
     * @brief Read an emulated attribute for Doorlock cluster 
     *        
     * This read_attribute overides the default read for emulated attributes.
     *
     * @param aPath
     * @param aEncoder
     * @return CHIP_ERROR
     */
    CHIP_ERROR read_attribute(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override
    {
        attribute_state_cache & cache = attribute_state_cache::get_instance();

        switch (aPath.mAttributeId)
        {
        case DoorLock::Attributes::OperatingMode::Id: {
            DoorLock::Attributes::OperatingMode::TypeInfo::Type operatingMode;

            if (!cache.get(aPath, operatingMode))
            {
                operatingMode = DoorLock::OperatingModeEnum::kNormal; 
            }
            return aEncoder.Encode(operatingMode);
        }
        case DoorLock::Attributes::SupportedOperatingModes::Id: {
            DoorLock::Attributes::SupportedOperatingModes::TypeInfo::Type supportedOperatingModes;
       
            if (!cache.get(aPath, supportedOperatingModes))
            {
                supportedOperatingModes = static_cast<DoorLock::Attributes::SupportedOperatingModes::TypeInfo::Type>(0XFFF6);
            }
            return aEncoder.Encode(supportedOperatingModes);
        }
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    };
};

} // namespace unify::matter_bridge

#endif // EMULATE_DOORLOCK_HPP
