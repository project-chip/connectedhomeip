/**
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "core/Types.h"

#ifndef MTRCommonCaseDeviceServerInitParamsProvider_h
#define MTRCommonCaseDeviceServerInitParamsProvider_h

/**
 * @brief Provides the ServerInitParams required to start the CastingApp, which in turn starts the Matter server
 */
class MTRCommonCaseDeviceServerInitParamsProvider : public matter::casting::support::ServerInitParamsProvider
{
private:
    // For this example, we'll use CommonCaseDeviceServerInitParams
    chip::CommonCaseDeviceServerInitParams serverInitParams;

public:
    chip::ServerInitParams * Get()
    {
        CHIP_ERROR err = serverInitParams.InitializeStaticResourcesBeforeServerInit();
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                            ChipLogError(AppServer, "Initialization of ServerInitParams failed %" CHIP_ERROR_FORMAT, err.Format()));
        return &serverInitParams;
    }
};

#endif /* MTRCommonCaseDeviceServerInitParamsProvider_h */
