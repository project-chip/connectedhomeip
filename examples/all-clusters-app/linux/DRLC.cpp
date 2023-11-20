/*
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/clusters/drlc-server/drlc-server.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DemandResponseLoadControl;
using Status = Protocols::InteractionModel::Status;

class OurDelegate : public DemandResponseLoadControl::Delegate
{
public:
    uint8_t GetNumberOfLoadControlPrograms() override;
    uint8_t GetNumberOfEventsPerProgram() override;
    uint8_t GetNumberOfTransitions() override;
    uint8_t GetDefaultRandomStart() override;
    CHIP_ERROR SetDefaultRandomStart(uint8_t aNewValue) override;
    uint8_t GetDefaultRandomDuration() override;
    CHIP_ERROR SetDefaultRandomDuration(uint8_t aNewValue) override;
    CHIP_ERROR GetLoadControlProgram(size_t aIndex, LoadControlProgram & aLoadControlProgram) override;
    CHIP_ERROR GetActiveEventID(size_t aIndex, FixedSpan<uint8_t, kEventIDSize> & aEventID) override;
    Status ReplaceLoadControlProgram(size_t aIndex, const Structs::LoadControlProgramStruct::DecodableType & aNewProgram) override;
    Status AddLoadControlProgram(const Structs::LoadControlProgramStruct::DecodableType & aNewProgram) override;
    Status RemoveLoadControlProgram(const ByteSpan & aProgramID) override;
    Status AddLoadControlEvent(const Structs::LoadControlEventStruct::DecodableType & aEvent) override;
    Status ClearLoadControlEvents() override;

protected:
    uint8_t mDefaultRandomStart    = 30;
    uint8_t mDefaultRandomDuration = 0;
};

static OurDelegate gDelegate;

static DemandResponseLoadControl::Instance</* ClearLoadControlEventsRequestSupported = */ true> gDRLCInstance(/* endpoint = */ 1,
                                                                                                              gDelegate);

void emberAfDemandResponseLoadControlClusterInitCallback(EndpointId endpoint)
{
    VerifyOrDie(endpoint == 1);

    VerifyOrDie(gDRLCInstance.Init() == CHIP_NO_ERROR);
}

uint8_t OurDelegate::GetNumberOfLoadControlPrograms()
{
    // Spec minimum for now
    return 5;
}

uint8_t OurDelegate::GetNumberOfEventsPerProgram()
{
    // Spec minimum for now
    return 10;
}

uint8_t OurDelegate::GetNumberOfTransitions()
{
    // Spec minimum for now
    return 3;
}

uint8_t OurDelegate::GetDefaultRandomStart()
{
    return mDefaultRandomStart;
}

CHIP_ERROR OurDelegate::SetDefaultRandomStart(uint8_t aNewValue)
{
    // TODO: Need to store in persistent storage, or change cluster to handle that.
    mDefaultRandomStart = aNewValue;
    return CHIP_NO_ERROR;
}

uint8_t OurDelegate::GetDefaultRandomDuration()
{
    return mDefaultRandomDuration;
}

CHIP_ERROR OurDelegate::SetDefaultRandomDuration(uint8_t aNewValue)
{
    // TODO: Need to store in persistent storage, or change cluster to handle that.
    mDefaultRandomDuration = aNewValue;
    return CHIP_NO_ERROR;
}

CHIP_ERROR OurDelegate::GetLoadControlProgram(size_t aIndex, LoadControlProgram & aLoadControlProgram)
{
    // TODO: Implement, complete with non-volatile storage that we have to
    // handle; the cluster can't help us here.
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

CHIP_ERROR OurDelegate::GetActiveEventID(size_t aIndex, FixedSpan<uint8_t, kEventIDSize> & aEventID)
{
    // TODO: Implement, complete with non-volatile storage that we have to
    // handle; the cluster can't help us here.
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

Status OurDelegate::ReplaceLoadControlProgram(size_t aIndex, const Structs::LoadControlProgramStruct::DecodableType & aNewProgram)
{
    // TODO: Implement, complete with non-volatile storage that we have to
    // handle; the cluster can't help us here.
    return Status::Failure;
}

Status OurDelegate::AddLoadControlProgram(const Structs::LoadControlProgramStruct::DecodableType & aNewProgram)
{
    // TODO: Implement, complete with non-volatile storage that we have to
    // handle; the cluster can't help us here.
    return Status::Failure;
}

Status OurDelegate::RemoveLoadControlProgram(const ByteSpan & aProgramID)
{
    // TODO: Implement, complete with non-volatile storage that we have to
    // handle; the cluster can't help us here.
    return Status::Failure;
}

Status OurDelegate::AddLoadControlEvent(const Structs::LoadControlEventStruct::DecodableType & aEvent)
{
    // TODO: Implement, complete with non-volatile storage that we have to
    // handle; the cluster can't help us here.
    return Status::Failure;
}

Status OurDelegate::ClearLoadControlEvents()
{
    // TODO: Implement, complete with non-volatile storage that we have to
    // handle; the cluster can't help us here.
    return Status::Failure;
}
