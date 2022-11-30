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

#ifndef MOCK_GROUP_TRANSLATOR_HPP
#define MOCK_GROUP_TRANSLATOR_HPP

#include "group_translator.hpp"
#include <unordered_map>

namespace unify::matter_bridge {
namespace Test {

class MockGroupTranslator : public unify::matter_bridge::group_translator
{
public:
    MockGroupTranslator(matter_data_storage & data_storage) : group_translator(data_storage) {}
    std::optional<unify_group_t> get_unify_group(const matter_group & group) override
    {
        std::unordered_map<uint16_t, uint16_t>::iterator iter = assigned_groups_id.find(group.group);
        if (iter != assigned_groups_id.end())
        {
            return iter->second;
        }
        return std::nullopt;
    }
    bool add_matter_group(const matter_group & group) override
    {
        mock_last_unify_assigned_id++;
        assigned_groups_id[group.group] = mock_last_unify_assigned_id;
        return true;
    }
    void remove_matter_group(const matter_group & group) override
    {
        mock_last_unify_assigned_id--;
        assigned_groups_id.erase(group.group);
    }

    std::unordered_map<uint16_t, uint16_t> assigned_groups_id;
    uint16_t mock_last_unify_assigned_id = 0;
};

} // namespace Test
} // namespace unify::matter_bridge

#endif