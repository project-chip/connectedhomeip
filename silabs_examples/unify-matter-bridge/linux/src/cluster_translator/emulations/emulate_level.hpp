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

#ifndef EMULATE_LevelControl_HPP
#define EMULATE_LevelControl_HPP

// Emulator interface
#include "emulator.hpp"

namespace unify::matter_bridge {

using namespace chip::app;
using namespace chip::app::Clusters;

class EmulateLevelControl : public EmulatorInterface
{
public:
    const char * emulated_cluster_name() const override { return "Level"; }

    chip::ClusterId emulated_cluster() const override { return LevelControl::Id; };

    std::vector<chip::AttributeId> emulated_attributes() const override
    {
        return { LevelControl::Attributes::MinLevel::Id, LevelControl::Attributes::CurrentLevel::Id };
    }

    CHIP_ERROR read_attribute(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override
    {
        attribute_state_cache & cache = attribute_state_cache::get_instance();

        // LevelControlType is a mandatory attribute we default it to None
        switch (aPath.mAttributeId)
        {
        case LevelControl::Attributes::StartUpCurrentLevel::Id: {
            LevelControl::Attributes::StartUpCurrentLevel::TypeInfo::Type startUpCurrentLevel;
            uint8_t start_up_level_value;

            if (cache.get(aPath, start_up_level_value))
            {
                if (start_up_level_value < 255)
                {
                    startUpCurrentLevel.SetNonNull(start_up_level_value);
                }
            }
            return aEncoder.Encode(startUpCurrentLevel);
        }
        case LevelControl::Attributes::MinLevel::Id: {
            LevelControl::Attributes::MinLevel::TypeInfo::Type min_level;
            if (cache.get(aPath, min_level))
            {
                if (min_level < 1)
                {
                    min_level = 1;
                }
                return aEncoder.Encode(min_level);
            }
            break;
        }
        case LevelControl::Attributes::CurrentLevel::Id: {
            LevelControl::Attributes::CurrentLevel::TypeInfo::Type current_level;
            uint8_t level_value;
            if (cache.get<uint8_t>(aPath, level_value))
            {
                if (level_value < 1)
                {
                    current_level.SetNonNull(1);
                }
                else if (level_value < 255)
                {
                    current_level.SetNonNull(level_value);
                } // 255 means not defined
            }
            return aEncoder.Encode(current_level);
        }
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    };
};

} // namespace unify::matter_bridge

#endif // EMULATE_LevelControl_HPP
