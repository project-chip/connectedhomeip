/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
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
