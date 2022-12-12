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
#include "group_translator.hpp"
#include "sl_log.h"
#include <app/server/Server.h>
#include <credentials/GroupDataProvider.h>
#include <iomanip>
#include <optional>
#include <regex>
#include <sstream>

using namespace chip;
using namespace chip::Credentials;

#define LOG_TAG "group_translator"

namespace unify::matter_bridge {

std::optional<unify_group_t> group_translator::get_unify_group(const matter_group & group)
{
    matter_data_storage::group_mapping group_info = { group.group, group.fabric_index };
    if (m_matter_data_storage.get_persisted_data(group_info))
    {
        return group_info.unify_group_id.value();
    }
    else
    {
        return std::nullopt;
    }
}

bool group_translator::add_matter_group(const matter_group & group)
{
    while (allocated_unify_groups.count(last_allocated_group))
    {
        last_allocated_group++;
        if (last_allocated_group == std::numeric_limits<unify_group_t>::max())
        {
            return false;
        }
    }
    matter_data_storage::group_mapping group_info = { group.group, group.fabric_index, last_allocated_group };
    return (m_matter_data_storage.persist_data(group_info));
}

void group_translator::remove_matter_group(const matter_group & group)
{
    auto ug = get_unify_group(group);
    if (ug)
    {
        matter_data_storage::group_mapping group_info = { group.group, group.fabric_index };
        m_matter_data_storage.remove_persisted_data(group_info);
    }
}

void group_translator::register_unify_group(const unify_group_t & group)
{
    allocated_unify_groups.insert(group);
}

void group_translator::on_mqtt_message_cb(const char * topic, const char * message, const size_t message_length)
{
    std::regex rgx("ucl/by-group"
                   "/([^/]*)" // group id
                   "/NodeList"
                   "/([^/]*)");
    std::smatch match;
    std::string topic_str(topic);
    if (!std::regex_search(topic_str, match, rgx))
    {
        return;
    }
    const std::string & group_id = match.str(1);
    const unify_group_t group    = std::stoi(group_id);
    group_translator::register_unify_group(group);
}

void group_translator::display_group_mapping(std::ostream& os)
{
    os<<"Display which Matter Groups correspond to which Unify Groups\n";
    os << std::setw(10) << "Matter Group |" << std::setw(10) << "Matter Fabric index |" << std::setw(10) << "Unify Group\n";
    GroupDataProvider * provider = GetGroupDataProvider();
    GroupDataProvider::GroupInfo group;
    matter_group matter_group_info;
    chip::FabricIndex fabric_index;
    for (const auto & fabric_index_iter : Server::GetInstance().GetFabricTable())
    {
        fabric_index         = fabric_index_iter.GetFabricIndex();
        auto group_info_iter = provider->IterateGroupInfo(fabric_index);
        if (group_info_iter != nullptr)
        {
            while (group_info_iter->Next(group))
            {
                matter_group_info.group                  = group.group_id;
                matter_group_info.fabric_index           = fabric_index;
                std::optional<unify_group_t> unify_group = group_translator::get_unify_group(matter_group_info);
                if (unify_group.has_value())
                {
                    os <<std::setw(10) << matter_group_info.group <<"|"<< std::setw(10) << matter_group_info.fabric_index
                       <<"|"<< std::setw(10) << unify_group.value()<<"\n";
                }
            }
            group_info_iter->Release();
        }
    }
}
} // namespace unify::matter_bridge