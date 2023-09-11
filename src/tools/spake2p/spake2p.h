/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
* SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file defines the public API symbols, constants, and interfaces
 *      for the 'spakep2' command line tool.
 */

#pragma once

#include <CHIPVersion.h>
#include <lib/support/Base64.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>

#define COPYRIGHT_STRING "SPDX-FileCopyrightText: 2022 Project CHIP Authors\n"

extern bool Cmd_GenVerifier(int argc, char * argv[]);
