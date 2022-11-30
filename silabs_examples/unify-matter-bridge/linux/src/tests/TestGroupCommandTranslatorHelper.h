/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
 * Helper module for Group Command Translator Test to use the TestContext framework.
 */
#include "attribute_translator_interface.hpp"

namespace unify::matter_bridge {
class GroupClusterAttributeTranslatorHelper : public attribute_translator_interface
{
public:
    GroupClusterAttributeTranslatorHelper(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::Groups::Id,
                                       "group_cluster_attribute_handler_helper"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override
    {
        return CHIP_NO_ERROR;
    };
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override
    {
        return CHIP_NO_ERROR;
    };

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override{};

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "Groups" }); }

    UnifyMqtt & m_unify_mqtt;
};
} // namespace unify::matter_bridge