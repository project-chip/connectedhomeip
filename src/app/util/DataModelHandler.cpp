/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 *   This file implements the handler for data model messages.
 */

#include <app/util/DataModelHandler.h>

#if __has_include(<zap-generated/endpoint_config.h>)
#define USE_ZAP_CONFIG 1
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#endif
#include <lib/support/logging/CHIPLogging.h>

using namespace ::chip;

void InitDataModelHandler()
{
#ifdef USE_ZAP_CONFIG
    ChipLogProgress(Zcl, "Using ZAP configuration...");
    emberAfEndpointConfigure();
    emberAfInit();
#endif
}
