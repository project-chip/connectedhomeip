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
#include "matter_data_storage.hpp"
#include "sl_log.h"
#include <iostream>
#include <sstream>
#include <string>

constexpr const char * LOG_TAG = "matter_data_storage";

namespace unify::matter_bridge {

matter_data_storage & matter_data_storage::instance()
{
    static matter_data_storage s;
    return s;
}

// Persist the assigned dynamic endpoint mapping
template <>
bool matter_data_storage::persist_data(endpoint_mapping & key_value)
{
    if (!key_value.matter_endpoint.has_value())
    {
        sl_log_info(LOG_TAG, "The matter endpoint value is not provided.");
        return false;
    }
    std::string key = std::string(key_value.unify_unid) + "-" + std::to_string(key_value.unify_endpoint);
    if (chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Put(key.c_str(), key_value.matter_endpoint.value()) ==
        CHIP_NO_ERROR)
    {
        return true;
    }
    return false;
}

// Persist the group mapping
template <>
bool matter_data_storage::persist_data(group_mapping & key_value)
{
    if (!key_value.unify_group_id.has_value())
    {
        sl_log_info(LOG_TAG, "The unify group id is not provided");
        return false;
    }
    std::string key = std::to_string(key_value.matter_group_id);
    if (chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Put(key.c_str(), key_value.unify_group_id.value()) ==
        CHIP_NO_ERROR)
    {
        return true;
    }
    return false;
}

// Get the persisted dynamic endpoint
template <>
bool matter_data_storage::get_persisted_data(endpoint_mapping & key_value)
{
    std::string key = std::string(key_value.unify_unid) + "-" + std::to_string(key_value.unify_endpoint);
    chip::EndpointId matter_endpoint;
    if (chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Get(key.c_str(), &matter_endpoint) == CHIP_NO_ERROR)
    {
        key_value.matter_endpoint = matter_endpoint;
        return true;
    }
    return false;
}

// Get the persisted group mapping
template <>
bool matter_data_storage::get_persisted_data(group_mapping & key_value)
{
    std::string key = std::to_string(key_value.matter_group_id);
    uint16_t unify_group_id;
    if (chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Get(key.c_str(), &unify_group_id) == CHIP_NO_ERROR)
    {
        key_value.unify_group_id = unify_group_id;
        return true;
    }
    return false;
}

// remove the persisted dynamic endpoint mapping
template <>
void matter_data_storage::remove_persisted_data(endpoint_mapping & key)
{
    std::string keys = std::string(key.unify_unid) + "-" + std::to_string(key.unify_endpoint);
    chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Delete(keys.c_str());
}

// remove the persisted group mapping
template <>
void matter_data_storage::remove_persisted_data(group_mapping & key)
{
    std::string keys = std::to_string(key.matter_group_id);
    chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Delete(keys.c_str());
}

} // namespace unify::matter_bridge