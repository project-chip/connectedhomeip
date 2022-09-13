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
 * @defgroup attribute_state_cache
 * @brief The module cache the attribute states in a memory and provides
 *        the state when a user of the API request the attribute state using
 *        ConcreteAttributePath.
 *
 * @{
 */

#ifndef ATTRIBUTE_STATE_CASH_H
#define ATTRIBUTE_STATE_CASH_H

#include <stdexcept>
#include <map>
#include <variant>
#include <string>
#include <iostream>

#include "matter.h"
#include <lib/core/Optional.h>

using value_t
  = std::variant<bool, uint8_t, uint16_t, uint32_t, uint64_t, int, std::string>;
using AttrPath_t = chip::app::ConcreteAttributePath;

class attribute_state_cache
{
  public:
  attribute_state_cache() {};
  ~attribute_state_cache() {};

  // Get the attribute_state_cache class instance
  static attribute_state_cache &get_instance();

  /**
   * @brief load the attribute state to a memory
   * 
   * @param attributePath  A representation of a concrete attribute path.
   * @param data   The attribute state value.
   */
  template<typename T> void set(const AttrPath_t &attributePath, const T &data)
  {
    attribute_state_container[attributePath] = data;
  }

  /**
   * @brief Get the attribute state from a memory
   * 
   * @param attributePath  A representation of a concrete attribute path
   */
  template<typename T> const T &get(const AttrPath_t &attributePath) const
  {
    auto iter = attribute_state_container.find(attributePath);
    if (iter != attribute_state_container.end()) {
      return std::get<T>(iter->second);
    } else {
      throw std::out_of_range(
        "The attribute path is not in attribute_state_container container");
    }
  }

  private:
  // Attribute State Container
  std::map<const AttrPath_t, value_t> attribute_state_container;
};

#endif  //ATTRIBUTE_STATE_CASH_H
/** @} end attribute_state_cache */
