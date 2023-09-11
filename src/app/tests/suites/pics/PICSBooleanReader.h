/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Declaration of PICS Boolean Reader, a class that read and parse
 *        a file with PICS Code and their enabled/disabled state.
 */

#include <map>
#include <string>

class PICSBooleanReader
{
public:
    static std::map<std::string, bool> Read(std::string filepath);
};
