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

#ifndef MOCK_MATTER_DATA_STORAGE_HPP
#define MOCK_MATTER_DATA_STORAGE_HPP

#include "matter_data_storage.hpp"

namespace unify::matter_bridge {
namespace Test {

class MockMatterDataStorage : public unify::matter_bridge::matter_data_storage
{
public:
    MockMatterDataStorage() : matter_data_storage() {}
    bool persist_data(group_mapping & key_value) override
    {
        last_assigned_group_id = key_value.unify_group_id.value();
        return true;
    }

    bool get_persisted_data(group_mapping & key_value) override 
    {
        key_value.unify_group_id = last_assigned_group_id;
        return true;
    }
    uint16_t last_assigned_group_id;
};

} // namespace Test
} // namespace unify::matter_bridge

#endif