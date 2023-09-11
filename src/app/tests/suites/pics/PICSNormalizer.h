/**
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Declaration of a method that normalizes a PICS code.
 */

#include <string>

class PICSNormalizer
{
public:
    static std::string Normalize(std::string code);
};
