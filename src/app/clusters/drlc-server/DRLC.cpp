/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#define DLRC_MAXEVENTS_PER_PROGRAM       10               // per spec min for now
#define DLRC_MAXLOADCONTROLPROGRAMS      5                // per spec min for now
#define DLRC_MAXEVENTTRANSITIONS         3                // per spec min for now

static LoadControlProgramStruct gLoadControlPrograms[DRLC_MAXPROGRAMS];             // about 256 bytes - our smallest underling processor has 390K
static LoadControlEventStruct   gLoadControlEvents[DLRC_MAXEVENTS_PER_PROGRAM];     // about 256 bytes - our smallest underling processor has 390K

static OurDelegate gDelegate;

static DemandResponseLoadControl::Instance</* ClearLoadControlEventsRequestSupported = */ true> gDRLCInstance(/* endpoint = */ 1,
                                                                                                              gDelegate);
void emberAfDemandResponseLoadControlClusterInitCallback(EndpointId endpoint)
{
    VerifyOrDie(endpoint == 1);

    VerifyOrDie(gDRLCInstance.Init() == Status::Success);
}

uint8_t OurDelegate::GetNumberOfLoadControlPrograms()
{
    // Spec max for now
    return DLRC_MAXLOADCONTROLPROGRAMS;
}

uint8_t OurDelegate::GetNumberOfEventsPerProgram()
{
    // Spec max for now
    return DLRC_MAXEVENTS_PER_PROGRAM;
}

uint8_t OurDelegate::GetNumberOfTransitions()
{
    // Spec minimum for now
    return DLRC_MAXEVENTTRANSITIONS;
}

uint8_t OurDelegate::GetDefaultRandomStart()
{
    return mDefaultRandomStart;
}

CHIP_ERROR OurDelegate::SetDefaultRandomStart(uint8_t aNewValue)
{
    // TODO: Implement - move to non-vol store
    mDefaultRandomStart = aNewValue;
    return Status::Success;
}

uint8_t OurDelegate::GetDefaultRandomDuration()
{
    return mDefaultRandomDuration;
}

CHIP_ERROR OurDelegate::SetDefaultRandomDuration(uint8_t aNewValue)
{
    // TODO: Implement - move to non-vol store
    mDefaultRandomDuration = aNewValue;
    return Status::Success;
}

CHIP_ERROR OurDelegate::GetLoadControlProgram(size_t aIndex, LoadControlProgram & aLoadControlProgram)
{
    // TODO: Implement - move to non-vol store

    if( aIndex < DRLC_MAXPROGRAMS ) {
        aLoadControlProgram = gLoadControlPrograms[aIndex];
        return( Status::Success );
    }
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

CHIP_ERROR OurDelegate::GetActiveEventID(size_t aIndex, FixedSpan<uint8_t, kEventIDSize> & aEventID)
{
    if( aIndex < DLRC_MAXEVENTS_PER_PROGRAM ) {
        aEventID = ByteSpan( gLoadControlEvents[aIndex].eventID );
        return( Status::Success );
    }
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

Status OurDelegate::ReplaceLoadControlProgram(size_t aIndex, const Structs::LoadControlProgramStruct::DecodableType & aNewProgram)
{
    // TODO: Implement - move to non-vol store

    if( aIndex < DRLC_MAXPROGRAMS ) {
        gLoadControlPrograms[aIndex] = aNewProgram;
        gLoadControlPrograms[aIndex].InUse = true;
        return( Status::Success );
    }

    return Status::Failure;
}

Status OurDelegate::AddLoadControlProgram(const Structs::LoadControlProgramStruct::DecodableType & aNewProgram)
{
    // TODO: Implement - move to non-vol store
    for( int i = 0; i < DRLC_MAXPROGRAMS; i++ ) {
        if( gLoadControlPrograms[i].InUse == false ) {
            gLoadControlPrograms[i].InUse = true;
            gLoadControlPrograms[i] = aNewProgram;
            return( Status::Success );
        }
    }
    return Status::Failure;
}

Status OurDelegate::RemoveLoadControlProgram(const ByteSpan & aProgramID)
{
    // TODO: Implement, complete with non-volatile storage
    for( int i = 0; i < DRLC_MAXPROGRAMS; i++ ) {
        if( aProgramID.data_equal( gLoadControlPrograms[i].programID ) {
            gLoadControlPrograms[i].InUse = false;
            return( Status::Success );
        }
    }
    return Status::Failure;
}

Status OurDelegate::AddLoadControlEvent(const Structs::LoadControlEventStruct::DecodableType & aEvent)
{
    // TODO: Implement, complete with non-volatile storage
    for( int i = 0; i < DLRC_MAXEVENTS_PER_PROGRAM; i++ ) {
        if( gLoadControlEvents[i].InUse == false ) {
            gLoadControlEvents[i].InUse = true;
            gLoadControlEvents[i] = aEvent;
            return( Status::Success );
        }
    }
    return Status::Failure;
}

Status OurDelegate::ClearLoadControlEvents()
{
    // TODO: Implement, complete with non-volatile storage
    for( int i = 0; i < DLRC_MAXEVENTS_PER_PROGRAM; i++ ) {
        gLoadControlEvents[i].InUse = false;
    }
    return( Status::Success );
}
