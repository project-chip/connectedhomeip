/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/valve-configuration-and-control-server/valve-configuration-and-control-cluster-logic.h>
#include <app/clusters/valve-configuration-and-control-server/valve-configuration-and-control-delegate.h>
#include <app/clusters/valve-configuration-and-control-server/valve-configuration-and-control-server-disco.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/KvsPersistentStorageDelegate.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ValveConfigurationAndControl;
namespace {
class PrintOnlyDelegate : public NonLevelControlDelegate
{
public:
    PrintOnlyDelegate(EndpointId endpoint) : mEndpoint(endpoint) {}
    CHIP_ERROR HandleOpenValve(ValveStateEnum & currentState, BitMask<ValveFaultBitmap> & valveFault) override
    {
        ChipLogProgress(NotSpecified, "VALVE IS OPENING!!!!!");
        state        = ValveStateEnum::kOpen;
        currentState = state;
        return CHIP_NO_ERROR;
    }
    ValveStateEnum GetCurrentValveState() override { return state; }
    CHIP_ERROR HandleCloseValve(ValveStateEnum & currentState, BitMask<ValveFaultBitmap> & valveFault) override
    {
        ChipLogProgress(NotSpecified, "VALVE IS CLOSING!!!!!");
        state        = ValveStateEnum::kClosed;
        currentState = state;
        return CHIP_NO_ERROR;
    }

private:
    ValveStateEnum state = ValveStateEnum::kClosed;
    EndpointId mEndpoint;
};

class NonLevelValveEndpoint
{
public:
    NonLevelValveEndpoint(EndpointId endpoint) :
        mEndpoint(endpoint), mContext(mEndpoint, storage), mDelegate(mEndpoint), mLogic(mDelegate, mContext),
        mInterface(mEndpoint, mLogic)
    {}
    CHIP_ERROR Init()
    {
        ReturnErrorOnFailure(mLogic.Init(kConformance, kInitParams));
        ReturnErrorOnFailure(mInterface.Init());
        return CHIP_NO_ERROR;
    }

private:
    const ClusterConformance kConformance = {
        .featureMap = 0, .supportsDefaultOpenLevel = false, .supportsValveFault = false, .supportsLevelStep = false
    };
    const ClusterInitParameters kInitParams = { .currentState = DataModel::MakeNullable(ValveStateEnum::kClosed),
                                                .currentLevel = DataModel::NullNullable,
                                                .valveFault   = 0,
                                                .levelStep    = 1 };
    EndpointId mEndpoint;
    KvsPersistentStorageDelegate storage;
    MatterContext mContext;
    PrintOnlyDelegate mDelegate;
    ClusterLogic mLogic;
    Interface mInterface;
};

NonLevelValveEndpoint ep1(1);
NonLevelValveEndpoint ep2(2);
NonLevelValveEndpoint ep3(3);
NonLevelValveEndpoint ep4(4);
NonLevelValveEndpoint ep5(5);
NonLevelValveEndpoint ep6(6);
} // namespace

void ApplicationInit()
{
    ChipLogError(NotSpecified, "App init!!!");
    ep1.Init();
    ep2.Init();
    ep3.Init();
    ep4.Init();
    ep5.Init();
    ep6.Init();
}

void ApplicationShutdown()
{
    ChipLogDetail(NotSpecified, "ApplicationShutdown()");
}

int main(int argc, char * argv[])
{
    if (ChipLinuxAppInit(argc, argv) != 0)
    {
        return -1;
    }

    ChipLinuxAppMainLoop();
    return 0;
}
