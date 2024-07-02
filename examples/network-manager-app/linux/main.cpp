/*
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <AppMain.h>
#include <app/clusters/wifi-network-management-server/wifi-network-management-server.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/Span.h>

#if MATTER_ENABLE_UBUS
#include "UbusManager.h"
#endif // MATTER_ENABLE_UBUS

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

ByteSpan ByteSpanFromCharSpan(CharSpan span)
{
    return ByteSpan(Uint8::from_const_char(span.data()), span.size());
}

void ApplicationInit()
{
    WiFiNetworkManagementServer::Instance().SetNetworkCredentials(ByteSpanFromCharSpan("MatterAP"_span),
                                                                  ByteSpanFromCharSpan("Setec Astronomy"_span));
}

class ApplicationMainLoop final : public DefaultAppMainLoopImplementation
{
public:
    void RunMainLoop() override;
    int Status() { return mStarted ? 0 : 1; }

private:
    bool mStarted = false;
};

void ApplicationMainLoop::RunMainLoop()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

#if MATTER_ENABLE_UBUS
    UbusManager ubus;
    SuccessOrExit(err = ubus.Init());
#endif // MATTER_ENABLE_UBUS

    mStarted = true;
    DefaultAppMainLoopImplementation::RunMainLoop();
    return;

exit:
    __attribute__((unused)); // label may be unused
    ChipLogError(Zcl, "Failed to start application run loop: %" CHIP_ERROR_FORMAT, err.Format());
}

void ApplicationShutdown() {}

int main(int argc, char * argv[])
{
    int ret = ChipLinuxAppInit(argc, argv);
    VerifyOrReturnValue(ret == 0, ret);

    ApplicationMainLoop application;
    ChipLinuxAppMainLoop(&application);
    return application.Status();
}
