/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *          Provides the implementation of the DeviceControlServer object.
 */

#include <platform/internal/DeviceControlServer.h>

#include <platform/ConfigurationManager.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

DeviceControlServer DeviceControlServer::sInstance;

CHIP_ERROR DeviceControlServer::ArmFailSafe(uint16_t expiryLengthSeconds)
{
    // TODO
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceControlServer::DisarmFailSafe()
{
    // TODO
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceControlServer::CommissioningComplete()
{
    // TODO
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceControlServer::SetRegulatoryConfig(uint8_t location, const char * countryCode, uint64_t breadcrumb)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = ConfigurationMgr().StoreRegulatoryLocation(location);
    SuccessOrExit(err);

    err = ConfigurationMgr().StoreCountryCode(countryCode, strlen(countryCode));
    SuccessOrExit(err);

    err = ConfigurationMgr().StoreBreadcrumb(breadcrumb);
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "SetRegulatoryConfig failed with error: %s", ErrorStr(err));
    }

    return err;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
