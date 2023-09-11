/*
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

namespace chip {
namespace app {
namespace DataModel {

//
// This type exists purely as a means to
// achieve normalization and consistency in templated APIs
// where it is possible to accept a valid data model type as
// well as not have one either. In the latter case, this type
// can be used to convey 'no type'.
//
struct NullObjectType
{
};

} // namespace DataModel
} // namespace app
} // namespace chip
