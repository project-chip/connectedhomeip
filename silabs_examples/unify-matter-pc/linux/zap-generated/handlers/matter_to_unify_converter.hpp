/*******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "app/data-model/NullObject.h"
#include "zap-types.h"
#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>

#define DEFAULT_ENUM_RETURN_VALUE 255

// Default translation
template <typename T>
T inline to_unify(const T& value)
{
    return value;
}

template <typename T1, typename T2>
T2 inline to_unify(const T1& value)
{
    return (T2)value;
}

string inline to_unify(const chip::Span<const char>& value)
{
    return std::string(value.data(), value.size());
}

string inline to_unify(const chip::Span<const unsigned char>& value)
{
    return std::string(reinterpret_cast<const char*>(value.data()), value.size());
}

template <>
uint8_t inline to_unify(const chip::app::Clusters::OnOff::DelayedAllOffEffectVariantEnum& value)
{
    using namespace chip::app::Clusters::OnOff;
    return static_cast<uint8_t>(value);
}
template <>
uint8_t inline to_unify(const chip::app::Clusters::OnOff::DyingLightEffectVariantEnum& value)
{
    using namespace chip::app::Clusters::OnOff;
    return static_cast<uint8_t>(value);
}
template <>
OffWithEffectEffectIdentifier inline to_unify(const chip::app::Clusters::OnOff::EffectIdentifierEnum& value)
{
    using namespace chip::app::Clusters::OnOff;
    switch (value) {
    case EffectIdentifierEnum::kDelayedAllOff:
        return ZCL_OFF_WITH_EFFECT_EFFECT_IDENTIFIER_DELAYED_ALL_OFF;
    case EffectIdentifierEnum::kDyingLight:
        return ZCL_OFF_WITH_EFFECT_EFFECT_IDENTIFIER_DYING_LIGHT;
    default:
        return static_cast<OffWithEffectEffectIdentifier>(DEFAULT_ENUM_RETURN_VALUE);
    }
}
template <>
OnOffStartUpOnOff inline to_unify(const chip::app::Clusters::OnOff::StartUpOnOffEnum& value)
{
    using namespace chip::app::Clusters::OnOff;
    switch (value) {
    case StartUpOnOffEnum::kOff:
        return ZCL_ON_OFF_START_UP_ON_OFF_SET_ON_OFF_TO0;
    case StartUpOnOffEnum::kOn:
        return ZCL_ON_OFF_START_UP_ON_OFF_SET_ON_OFF_TO1;
    case StartUpOnOffEnum::kToggle:
        return ZCL_ON_OFF_START_UP_ON_OFF_TOGGLE_PREVIOUS_ON_OFF;
    case StartUpOnOffEnum::kUnknownEnumValue:
        return ZCL_ON_OFF_START_UP_ON_OFF_SET_PREVIOUS_ON_OFF;
    default:
        return static_cast<OnOffStartUpOnOff>(DEFAULT_ENUM_RETURN_VALUE);
    }
}
