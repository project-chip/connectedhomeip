#include <app/util/af.h>

#include <app/util/af-event.h>
#include <app/util/attribute-storage.h>

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/command-id.h>
#include <app-common/zap-generated/enums.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/error-mapping.h>
#include <lib/core/CHIPEncoding.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

Protocols::InteractionModel::Status MatterThermostatUserInterfaceConfigurationClusterServerPreAttributeChangedCallback(
    const app::ConcreteAttributePath & attributePath, EmberAfAttributeType attributeType, uint16_t size, uint8_t * value)
{
    if (size != 0)
    {
        if (ThermostatUserInterfaceConfiguration::Attributes::TemperatureDisplayMode::Id == attributePath.mAttributeId)
        {
            EmberAfTemperatureDisplayMode mode = static_cast<EmberAfTemperatureDisplayMode>(chip::Encoding::Get8(value));
            if ((EMBER_ZCL_TEMPERATURE_DISPLAY_MODE_CELSIUS != mode) && (EMBER_ZCL_TEMPERATURE_DISPLAY_MODE_FAHRENHEIT != mode))
            {
                return Protocols::InteractionModel::Status::Failure;
            }
        }
        else if (ThermostatUserInterfaceConfiguration::Attributes::KeypadLockout::Id == attributePath.mAttributeId)
        {
            EmberAfKeypadLockout lockout = static_cast<EmberAfKeypadLockout>(chip::Encoding::Get8(value));
            if ((EMBER_ZCL_KEYPAD_LOCKOUT_NO_LOCKOUT != lockout) && (EMBER_ZCL_KEYPAD_LOCKOUT_LEVEL_ONE_LOCKOUT != lockout) &&
                (EMBER_ZCL_KEYPAD_LOCKOUT_LEVEL_TWO_LOCKOUT != lockout) &&
                (EMBER_ZCL_KEYPAD_LOCKOUT_LEVEL_THREE_LOCKOUT != lockout) &&
                (EMBER_ZCL_KEYPAD_LOCKOUT_LEVEL_FOUR_LOCKOUT != lockout) && (EMBER_ZCL_KEYPAD_LOCKOUT_LEVELFIVE_LOCKOUT != lockout))
            {
                return Protocols::InteractionModel::Status::Failure;
            }
        }
        else if (ThermostatUserInterfaceConfiguration::Attributes::ScheduleProgrammingVisibility::Id == attributePath.mAttributeId)
        {
            uint8_t prog = chip::Encoding::Get8(value);
            if (prog > 1)
            {
                return Protocols::InteractionModel::Status::Failure;
            }
        }
    }

    return Protocols::InteractionModel::Status::Success;
}
