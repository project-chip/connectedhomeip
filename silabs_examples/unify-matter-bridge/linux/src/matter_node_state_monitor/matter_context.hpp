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

#ifndef MATTER_CONTEXT_HPP
#define MATTER_CONTEXT_HPP

#include "context_builder.hpp"
#include "matter.h"

// Device Version for dynamic endpoints:
#define DEVICE_VERSION_DEFAULT 1
#define DEVICE_TYPE_BRIDGED_NODE 0x0013

namespace unify::matter_bridge
{
/**
     * @brief This context object holds the actual `EmberAfEndpointType` that is
     * passed to the matter API. Next to that it holds the lifetimes of memory
     * that is used inside of this type. When this object goes out of scope, all
     * memory gets released. Therefore its critical that this object lives
     * longer as the references of the matter_endpoint inside the matter API.
     *
     * use a `ember_context_builder` to setup this object.
     */
class matter_endpoint_context :
  public lifetime_context<EmberAfEndpointType,
                          EmberAfCluster,
                          EmberAfAttributeMetadata,
                          chip::CommandId>
{
  public:
  matter_endpoint_context() = default;

  /**
   * @brief Storage that will be passed on a matter declare dynamic endpoint.
   * Its related to the number of clusters being declared.
   */
  const chip::Span<chip::DataVersion> data_version_span() const
  {
    using BaseType = lifetime_context<EmberAfEndpointType,
                                      EmberAfCluster,
                                      EmberAfAttributeMetadata,
                                      chip::CommandId>;
    // This should be a const operation. For now we resize the data versions
    // buffer here. Ideally this should have been done in the last non const
    // operation of the context object.
    auto& versions = (const_cast<matter_endpoint_context *>(this))->data_versions;
    std::fill_n(std::back_inserter(versions), BaseType::operator*()->clusterCount
      , 0);

    return chip::Span<chip::DataVersion>(
      const_cast<chip::DataVersion *>(versions.data()),
      versions.size()*sizeof(chip::DataVersion) );
  }

  const chip::Span<const EmberAfDeviceType> device_type_span(uint16_t matter_type) const
  {
    auto& device_types = (const_cast<matter_endpoint_context *>(this))->bridgedDeviceTypes;
    device_types[0].deviceId = matter_type;
    return chip::Span<const EmberAfDeviceType>(bridgedDeviceTypes);
  }

  private:
  std::vector<chip::DataVersion> data_versions;
  EmberAfDeviceType bridgedDeviceTypes[2] = { { 0, DEVICE_VERSION_DEFAULT },
                                            { DEVICE_TYPE_BRIDGED_NODE, DEVICE_VERSION_DEFAULT } };
};

}  // namespace unify::matter_bridge
#endif