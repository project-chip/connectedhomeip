/*
 *
 * SPDX-FileCopyrightText: 2020-2023 Project CHIP Authors
 * SPDX-FileCopyrightText: 2013-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file contains definitions for working with data encoded in CHIP TLV format.
 *
 *      CHIP TLV (Tag-Length-Value) is a generalized encoding method for simple structured data. It
 *      shares many properties with the commonly used JSON serialization format while being considerably
 *      more compact over the wire.
 */

#pragma once

#include "TLVCommon.h"

#include "TLVBackingStore.h"
#include "TLVReader.h"
#include "TLVUpdater.h"
#include "TLVWriter.h"
