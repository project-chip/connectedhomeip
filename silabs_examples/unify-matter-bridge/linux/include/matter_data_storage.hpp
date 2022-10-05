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
 * @defgroup matter_data_storage
 * @brief the module persist the dynamic endpoint mapping
 *        from unify unid and endpoint.
 *
 * @{
 */

#ifndef MATTER_DATA_STORAGE_HPP
#define MATTER_DATA_STORAGE_HPP

#include "matter.h"
#include <optional>
#include <platform/KeyValueStoreManager.h>

namespace unify::matter_bridge {
class matter_data_storage
{
public:
    struct endpoint_mapping
    {
        const char * unify_unid;
        uint8_t unify_endpoint;
        std::optional<chip::EndpointId> matter_endpoint;
    };
    struct group_mapping
    {
        uint16_t matter_group_id;
        std::optional<uint16_t> unify_group_id;
    };
    /**
     * @brief Persist data
     *
     * @param key_value a struct that contain the key and value,
     *                  that should be persisted to the data storage
     * @return true if the data is persisted
     * @return false if the data is not persisted
     */
    template <typename T>
    bool persist_data(T & key_value);

    /**
     * @brief Get the persisted dynamic endpoint
     *
     * @param key_value a struct that contain the key and value
     *                  that the persisted dat copy over
     * @return true if value is written on the value field of
     *         the key_value struct
     * @return false if the key is not found in the data storage
     */
    template <typename T>
    bool get_persisted_data(T & key_value);
    /**
     * @brief remove the persisted data
     *
     * @param key key
     */
    template <typename T>
    void remove_persisted_data(T & key);

    static matter_data_storage & instance();
};

} // namespace unify::matter_bridge
#endif // MATTER_DATA_STORAGE_HPP
/** @} end matter_data_storage */
