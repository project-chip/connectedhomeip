/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "Base38.h"

#include <string>
#include <vector>

namespace chip {

CHIP_ERROR base38Decode(std::string base38, std::vector<uint8_t> & out);

} // namespace chip
