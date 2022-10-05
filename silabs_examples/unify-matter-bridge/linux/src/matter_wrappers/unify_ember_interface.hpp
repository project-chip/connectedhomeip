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

#include "sl_log.h"
#include <app/util/af.h>
#include <app/util/attribute-storage.h>

class UnifyEmberInterface
{
protected:
    chip::EndpointId last_returned_available_endpoint_id = FIXED_ENDPOINT_COUNT;
    const char * LOG_TAG                                 = "Unify Ember Interface";

public:
    // Class setup and teardown
    UnifyEmberInterface(){};
    virtual ~UnifyEmberInterface(){};

    // Wrapper logic improving the interface

    /**
     * @brief Get next available dynamic endpoint.
     *
     * If more than 65535 - FIXED_ENDPOINT_COUNT is registered the function will
     * return chip::kInvalidEndpointId.
     *
     * @return chip::EndpointId
     */
    chip::EndpointId getNextDynamicAvailableEndpointIndex()
    {
        // Initially increment the endpoint count by 1 to skip the last fixed
        // endpoint
        chip::EndpointId endpoint_id = static_cast<chip::EndpointId>(last_returned_available_endpoint_id + 1);
        for (; endpoint_id < MAX_ENDPOINT_COUNT; endpoint_id++)
        {
            if (emberAfGetDynamicIndexFromEndpointUnify(endpoint_id) == chip::kInvalidEndpointId &&
                endpoint_id < MAX_ENDPOINT_COUNT)
            {
                last_returned_available_endpoint_id = endpoint_id;
                return endpoint_id;
            }
        }

        // If we have used all our endpoints or the incremented endpoint is
        // already in use, search for a valid endpoint (This is not good for
        // scale aka large networks).
        sl_log_debug(LOG_TAG, "Warning: searching through all endpoints to find a free one");
        chip::EndpointId endpoint_count_start = emberAfFixedEndpointCountUnify();
        for (chip::EndpointId i = endpoint_count_start; i < MAX_ENDPOINT_COUNT; i++)
        {
            if (emberAfGetDynamicIndexFromEndpointUnify(i) == chip::kInvalidEndpointId)
            {
                return i;
            }
        }
        return chip::kInvalidEndpointId;
    }

    // Matter ember interface stack wrappers (Please add wrappers here if functions are missing)
    // src/app/util/attribute-storage.h
    virtual uint16_t emberAfGetDynamicIndexFromEndpointUnify(chip::EndpointId id) { return emberAfGetDynamicIndexFromEndpoint(id); }
    virtual EmberAfStatus emberAfSetDynamicEndpointUnify(uint16_t index, chip::EndpointId id, const EmberAfEndpointType * ep,
                                                         const chip::Span<chip::DataVersion> & dataVersionStorage,
                                                         chip::Span<const EmberAfDeviceType> deviceTypeList = {},
                                                         chip::EndpointId parentEndpointId = chip::kInvalidEndpointId)
    {
        EmberAfStatus emberStatus = emberAfSetDynamicEndpoint(index, id, ep, dataVersionStorage, deviceTypeList, parentEndpointId);
        return emberStatus;
    }
    virtual chip::EndpointId emberAfClearDynamicEndpointUnify(uint16_t index)
    {
        chip::EndpointId id = emberAfClearDynamicEndpoint(index);
        return id;
    }

    // src/app/util/af.h
    virtual bool emberAfEndpointEnableDisableUnify(chip::EndpointId endpoint, bool enabled)
    {
        return emberAfEndpointEnableDisable(endpoint, enabled);
    }
    virtual chip::EndpointId emberAfEndpointFromIndexUnify(uint16_t index) { return emberAfEndpointFromIndex(index); }
    virtual uint16_t emberAfFixedEndpointCountUnify(void) { return emberAfFixedEndpointCount(); };
};