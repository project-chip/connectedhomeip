/*******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#ifndef CHIP_TYPE_TRAITS_H
#define CHIP_TYPE_TRAITS_H

#include <lib/support/Span.h>
#include <lib/support/BitMask.h>
#include <app/data-model/Nullable.h>

namespace unify::matter_bridge {
    //This is template is used to compile time determine if type is a chip::Span<>
    // This is done using a SFINAE pattern
    template <typename T>
    struct is_span : std::false_type {};

    template <typename T>
    struct is_span<chip::Span<T>> : std::true_type {};

    template <typename T>
    struct is_bitmask : std::false_type {};
    template <typename T>
    struct is_bitmask<chip::BitMask<T>> : std::true_type {};

    template <typename T>
    struct is_nullable : std::false_type {};
    template <typename T>
    struct is_nullable<chip::app::DataModel::Nullable<T>> : std::true_type {};

}
#endif