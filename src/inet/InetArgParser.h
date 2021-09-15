/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2017 Nest Labs, Inc.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *      Support functions for parsing command-line arguments for Inet types
 *
 */

#pragma once

#include <lib/core/CHIPConfig.h>

#if CHIP_CONFIG_ENABLE_ARG_PARSER

#include <lib/support/CHIPArgParser.hpp>

#include <inet/IPAddress.h>

namespace chip {
namespace ArgParser {

bool ParseIPAddress(const char * str, chip::Inet::IPAddress & output);

} // namespace ArgParser
} // namespace chip

#endif // CHIP_CONFIG_ENABLE_ARG_PARSER
