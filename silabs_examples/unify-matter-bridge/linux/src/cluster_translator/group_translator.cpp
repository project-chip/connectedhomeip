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
#include "matter_data_storage.hpp"
#include <optional>
#include <sstream>
#include <sstream>
#include <regex>

namespace unify::matter_bridge
{

std::optional<unify_group_t>
  group_translator::get_unify_group(const matter_group &group)
{
  matter_data_storage::group_mapping group_info = {group.group};
  if (matter_data_storage::instance().get_persisted_data(group_info)) {
    return group_info.unify_group_id.value();
  } else {
    return std::nullopt;
  }
}

bool group_translator::add_matter_group(const matter_group &group)
{
  while (allocated_unify_groups.count(last_allocated_group)) {
    last_allocated_group++;
    if (last_allocated_group == std::numeric_limits<unify_group_t>::max()) {
      return false;
    }
  }
  matter_data_storage::group_mapping group_info
    = {group.group, last_allocated_group};
  matter_data_storage::instance().persist_data(group_info);
  return true;
}

void group_translator::remove_matter_group(const matter_group &group)
{
  auto ug = get_unify_group(group);
  if (ug) {
    matter_data_storage::group_mapping group_info = {group.group};
    matter_data_storage::instance().remove_persisted_data(group_info);
  }
}

void group_translator::register_unify_group(const unify_group_t &group)
{
  allocated_unify_groups.insert(group);
}

void group_translator::on_mqtt_message_cb(const char *topic,
                                          const char *message,
                                          const size_t message_length)
{
  std::regex rgx("ucl/by-group"
                 "/([^/]*)"  // group id
                 "/NodeList"
                 "/([^/]*)");
  std::smatch match;
  std::string topic_str(topic);
  if (!std::regex_search(topic_str, match, rgx)) {
    return;
  }
  const std::string &group_id = match.str(1);
  const unify_group_t group   = std::stoi(group_id);
  group_translator::register_unify_group(group);
}
}  // namespace unify::matter_bridge