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
 * @defgroup bridged_device_basic_info_attribute_translator
 * @brief The module collect all relevant information regarding
 *        Basic/NameAndLocation/State clusters from the unify node and map them to
 *        matter bridged device basic information cluster.
 *
 * @{
 */

#ifndef BRIDGED_DEVICE_BASIC_INFO_ATTRIBUTE_TRANSLATOR_HPP
#define BRIDGED_DEVICE_BASIC_INFO_ATTRIBUTE_TRANSLATOR_HPP

#include "attribute_translator_interface.hpp"

namespace unify::matter_bridge {
class BridgedDeviceBasicInfoAttributeAccess : public attribute_translator_interface
{
public:
    BridgedDeviceBasicInfoAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                          device_translator & dev_translator);
    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override
    {
        return std::vector<const char *>({ "Basic", "NameAndLocation" });
    }

private:
    uint32_t map_basic_cluster_attributes(const std::string & dot_dot_basic_attributes) const;
    static void unify_node_reachable_state_update(const bridged_endpoint & ep, matter_node_state_monitor::update_t state);
};

} // namespace unify::matter_bridge

#endif // BRIDGED_DEVICE_BASIC_INFO_ATTRIBUTE_TRANSLATOR_HPP
/** @} end bridged_device_basic_info_attribute_translator */
