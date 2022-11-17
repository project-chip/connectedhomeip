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

#ifndef MOCK_UNIFY_EMBER_INTERFACE_HPP
#define MOCK_UNIFY_EMBER_INTERFACE_HPP

#include "unify_ember_interface.hpp"

namespace unify::matter_bridge {
namespace Test {

class MockUnifyEmberInterface : public UnifyEmberInterface
{
public:
    MockUnifyEmberInterface() : UnifyEmberInterface(){};

    chip::EndpointId calledWithId = 101;
    uint16_t calledWithIndex;
    EmberAfEndpointType * calledWithEndpoint;
    chip::Span<const EmberAfDeviceType> calledWithDeviceTypeList = {};
    chip::EndpointId calledWithParentEndpointId                  = chip::kInvalidEndpointId;
    bool lastCalledEnabled;
    int calledNTimes = 0;

    // All the endpoints cleared
    std::vector<chip::EndpointId> cleared_endpoints;

    EmberAfStatus emberAfSetDynamicEndpointUnify(uint16_t index, chip::EndpointId id, const EmberAfEndpointType * ep,
                                                 const chip::Span<chip::DataVersion> & dataVersionStorage,
                                                 chip::Span<const EmberAfDeviceType> deviceTypeList,
                                                 chip::EndpointId parentEndpointId) override
    {
        calledWithIndex            = index;
        calledWithId               = id;
        calledWithEndpoint         = const_cast<EmberAfEndpointType *>(ep);
        calledWithDeviceTypeList   = deviceTypeList;
        calledWithParentEndpointId = parentEndpointId;
        return emberAfSetDynamicEndpoint(index, id, ep, dataVersionStorage, deviceTypeList, parentEndpointId);
    }

    uint16_t emberAfFixedEndpointCountUnify() override { return emberAfFixedEndpointCount(); }

    chip::EndpointId emberAfClearDynamicEndpointUnify(uint16_t index) override
    {
        chip::EndpointId id = emberAfClearDynamicEndpoint(index);
        if (id != chip::kInvalidEndpointId)
        {
            cleared_endpoints.push_back(id);
        }
        return id;
    }

    chip::EndpointId getLastReturnedAvailableEndpointId() { return last_returned_available_endpoint_id; }
};

} // namespace Test
} // namespace unify::matter_bridge
#endif
