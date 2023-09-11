/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 *   This file defines functions for taking configuration for the LwIP stack
 *   from the corresponding configuration on the tap interface.
 */

#include <vector>

int CollectTapAddresses(std::vector<char *> & addresses, const char * ifName);
