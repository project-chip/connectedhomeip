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
 * @brief This module provide override function for getting
 *        the feature map setting for supported clusters.
 *
 */

#ifndef FEATURE_MAP_ATTR_OVERRIDE_HPP
#define FEATURE_MAP_ATTR_OVERRIDE_HPP

#include "attribute_translator.hpp"

namespace unify::matter_bridge {
class ColorControllerAttributeAccessOverride : public ColorControlAttributeAccess
{
public:
    ColorControllerAttributeAccessOverride(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        ColorControlAttributeAccess(node_state_monitor, unify_mqtt)
    {}

protected:
    uint32_t get_feature_map_settings(const chip::app::ConcreteReadAttributePath & aPath) override;
};

class OnOffAttributeAccessOverride : public OnOffAttributeAccess
{
public:
    OnOffAttributeAccessOverride(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        OnOffAttributeAccess(node_state_monitor, unify_mqtt)
    {}

protected:
    uint32_t get_feature_map_settings(const chip::app::ConcreteReadAttributePath & aPath) override;
};

class LevelControlAttributeAccessOverride : public LevelControlAttributeAccess
{
public:
    LevelControlAttributeAccessOverride(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        LevelControlAttributeAccess(node_state_monitor, unify_mqtt)
    {}

protected:
    uint32_t get_feature_map_settings(const chip::app::ConcreteReadAttributePath & aPath) override;
};

} // namespace unify::matter_bridge
#endif // FEATURE_MAP_ATTR_OVERRIDE_HPP