/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2016-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file contains a preprocessor macro that resolves to a
 *      constant expression that evaluates to the size of an array
 *      element with alignment padding.
 */

#pragma once

#define CHIP_SYSTEM_ALIGN_SIZE(ELEMENT, ALIGNMENT) (((ELEMENT) + (ALIGNMENT) -1) & ~((ALIGNMENT) -1))
