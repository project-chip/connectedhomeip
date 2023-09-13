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
#include "sl_log.h"


namespace unify::matter_bridge {

using namespace chip::app;
using namespace chip::app::Clusters;

class EmulateDoorLock : public EmulatorInterface
{
private :
    nlohmann::json inline convert_to_json(const chip::Span<const unsigned char>& value)
    {
        return std::string(reinterpret_cast<const char*>(value.data()), value.size());
    }
public:
    const char * emulated_cluster_name() const override { return "DoorLock"; }

    chip::ClusterId emulated_cluster() const override { return DoorLock::Id; };

    std::vector<chip::AttributeId> emulated_attributes() const override
    {
        return { DoorLock::Attributes::OperatingMode::Id, DoorLock::Attributes::SupportedOperatingModes::Id };
    }

    std::vector<chip::CommandId> emulated_commands() const override
    {
        return { DoorLock::Commands::LockDoor::Id, DoorLock::Commands::UnlockDoor::Id };
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
        cluster_builder.incoming_commands.emplace_back(DoorLock::Commands::LockDoor::Id);
        cluster_builder.incoming_commands.emplace_back(DoorLock::Commands::UnlockDoor::Id);

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

    /**
     * @brief Handle an emulated command for Doorlock cluster
     *
     * @param handlerContext
     * @param cdata Holder for the emulated command data and payload
     * @return CHIP_ERROR
     */
    CHIP_ERROR command(CommandHandlerInterface::HandlerContext & handlerContext, emulated_cmd_payload & cdata)
    {
        // Doorlock cluster emulated command need further handling in command_translotor.
        cdata.cmd_emulation_completed = false;

        switch (handlerContext.mRequestPath.mCommandId)
        {
        case DoorLock::Commands::LockDoor::Id: {
            DoorLock::Commands::LockDoor::DecodableType data;
            cdata.cmd = "LockDoor"; // "LockDoor";
            if (DataModel::Decode(handlerContext.GetReader(), data) == CHIP_NO_ERROR) {
                if (data.PINCode.HasValue()) {
                    try {
                        cdata.payload["PINOrRFIDCode"] = convert_to_json(data.PINCode.Value());
                    } catch (std::exception& ex) {
                        sl_log_warning("Failed to add the command arguments value to json format: %s", ex.what());
                    }
                } else {
                    cdata.payload["PINOrRFIDCode"] = "";
                }
            }
            return CHIP_NO_ERROR;
        }
        case DoorLock::Commands::UnlockDoor::Id: {
            DoorLock::Commands::UnlockDoor::DecodableType data;
            cdata.cmd = "UnlockDoor"; // "UnlockDoor";
            if (DataModel::Decode(handlerContext.GetReader(), data) == CHIP_NO_ERROR) {
                if (data.PINCode.HasValue()) {
                    try {
                        cdata.payload["PINOrRFIDCode"] = convert_to_json(data.PINCode.Value());
                    } catch (std::exception & ex) {
                        sl_log_warning("Failed to add the command arguments value to json format: %s", ex.what());
                    }
                } else {
                    cdata.payload["PINOrRFIDCode"] = "";
                }
            }
            return CHIP_NO_ERROR;
        }
        }
        return CHIP_ERROR_NOT_IMPLEMENTED; 
    };
};

} // namespace unify::matter_bridge

#endif // EMULATE_DOORLOCK_HPP
