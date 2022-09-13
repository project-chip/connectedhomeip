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
 * @defgroup identify_attribute_translator
 * @brief components
 *
 * It parse and update attribute state update for identify cluster.
 * 
 * @{
 */

#ifndef IDENTIFY_ATTRIBUTE_TRANSLATOR_HPP
#define IDENTIFY_ATTRIBUTE_TRANSLATOR_HPP

#include "attribute_translator_interface.hpp"

namespace unify::matter_bridge
{
class IdentifyAttributeAccess : public attribute_translator_interface
{
  public:
  IdentifyAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::Identify::Id)
  {}
  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string &cluster,
                        const std::string &attribute,
                        const nlohmann::json &unify_value) override;
  std::vector<const char *> unify_cluster_names() const override
  {
    return std::vector<const char *>({"Identify"});
  }
};

}  // namespace unify::matter_bridge
#endif  //IDENTIFY_ATTRIBUTE_TRANSLATOR_HPP
/** @} end identify_attribute_translator */
