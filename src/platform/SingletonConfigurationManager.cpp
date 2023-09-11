/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Implements a getter and setter for a singleton ConfigurationManager object.
 */

#include <lib/support/CodeUtils.h>
#include <platform/ConfigurationManager.h>

namespace chip {
namespace DeviceLayer {

class ConfigurationManager;

namespace {

/** Singleton pointer to the ConfigurationManager implementation.
 */
ConfigurationManager * gInstance = nullptr;

} // namespace

ConfigurationManager & ConfigurationMgr()
{
    if (gInstance != nullptr)
    {
        return *gInstance;
    }

    return ConfigurationMgrImpl();
}

void SetConfigurationMgr(ConfigurationManager * configurationManager)
{
    if (configurationManager != nullptr)
    {
        gInstance = configurationManager;
    }
}

} // namespace DeviceLayer
} // namespace chip
