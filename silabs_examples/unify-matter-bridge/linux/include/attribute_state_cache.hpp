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

#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <variant>

#include "matter.h"
#include <algorithm>
#include <app/util/odd-sized-integers.h>
#include <lib/core/Optional.h>
#include <string.h>
#include <type_traits>
using value_t    = std::variant<bool, float, int8_t, uint8_t, int16_t, uint16_t, uint32_t, uint64_t, int, std::string,
                             std::vector<char>, std::vector<uint8_t>>;
using AttrPath_t = chip::app::ConcreteAttributePath;



namespace unify::matter_bridge {
    //This is template is used to compile time determine if type is a chip::Span<>
    // This is done using a SFINAE pattern
    template <typename T>
    struct is_span : std::false_type {};

    template <typename T>
    struct is_span<chip::Span<T>> : std::true_type {};
}

class attribute_state_cache
{
public:
    attribute_state_cache(){};
    ~attribute_state_cache(){};

    // Get the attribute_state_cache class instance
    static attribute_state_cache & get_instance();

    /**
     * @brief save the attribute state to a memory
     *
     * @param attributePath  A representation of a concrete attribute path.
     * @param data   The attribute state value.
     */
    template <typename T>
    void set(const AttrPath_t & attributePath, const T & data)
    {
        if constexpr (std::is_enum<T>::value)
        {
            attribute_state_container[attributePath] = static_cast<std::underlying_type_t<T>>(data);
        } else if constexpr (unify::matter_bridge::is_span<T>::value) {
            std::vector<uint8_t> v(data.size());
            std::copy(data.begin(), data.end(), std::back_inserter(v));
            set(attributePath, v);
        }
        else
        {
            attribute_state_container[attributePath] = data;
        }
    }


    /**
     * @brief Setter for Nullable
     * 
     * If the object does not have a value this is a NOP
     * 
     * @tparam T 
     * @param attributePath 
     * @param data 
     */
    template <typename T>
    void set(const AttrPath_t & attributePath, const chip::app::DataModel::Nullable<T> & data)
    {
        if (data.HasValidValue())
        {
            set(attributePath, data.Value());
        }
    }

    /**
     * @brief Get the attribute state from a memory, returns direct reference to the stored object.
     *
     * @param attributePath  A representation of a concrete attribute path
     */
    template <typename T>
    const T & get(const AttrPath_t & attributePath) const
    {
        auto iter = attribute_state_container.find(attributePath);
        if (iter != attribute_state_container.end())
        {
            return std::get<T>(iter->second);
        }
        else
        {
            throw std::out_of_range("The attribute path is not in attribute_state_container container");
        }
    }

    /**
     * @brief Generic getter for all all attribute types, the stored value will be 
     * copied to value
     * 
     * @tparam T 
     * @param attributePath 
     * @param value 
     * @return true 
     * @return false 
     */
    template <typename T>
    bool inline get(const AttrPath_t & attributePath, T & value) const
    {
        auto iter = attribute_state_container.find(attributePath);
        if (iter != attribute_state_container.end())
        {
            if constexpr (std::is_enum<T>::value)
            {
                auto v = std::get<std::underlying_type_t<T>>(iter->second);
                value  = static_cast<T>(v);
            }
            else if constexpr (std::is_array<T>::value)
            {
                return get_array(attributePath, value);
            } else if constexpr (unify::matter_bridge::is_span<T>::value) 
            {
                return false; //TODO this should really be a check for is_chip_span
            }
            else
            {
                value = std::get<T>(iter->second);
            }
            return true;
        }
        else
        {
            return false;
        }
    }


private:
    /**
     * @brief Getter for an array type
     * 
     * @tparam T 
     * @tparam N 
     * @param attributePath 
     * @return true 
     * @return false 
     */
    template <typename T, std::size_t N>
    bool inline get_array(const AttrPath_t & attributePath, T (&value)[N]) const
    {
        std::vector<T> v;
        if (get(attributePath, v))
        {
            if (v.size() == N)
            {
                std::copy(v.begin(), v.end(), &value[0]);
                return true;
            }
        }
        return false;
    }

    // Attribute State Container
    std::map<const AttrPath_t, value_t> attribute_state_container;
};

#endif // ATTRIBUTE_STATE_CASH_H
/** @} end attribute_state_cache */
