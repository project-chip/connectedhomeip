#include "rvc-device.h"

#include <string>

using namespace chip::app::Clusters;

void RvcDevice::Init()
{
//    mServiceAreaInstance.Init();
    mRunModeInstance.Init();
    mCleanModeInstance.Init();
    mOperationalStateInstance.Init();

    // hardcoded fill of SUPPORTED MAPS for prototyping
    uint8_t supportedMapId_XX  = 3;
    uint8_t supportedMapId_YY  = 245;

    mServiceAreaInstance.AddSupportedMap(supportedMapId_XX, CharSpan::fromCharString("My Map XX"));
    mServiceAreaInstance.AddSupportedMap(supportedMapId_YY, CharSpan::fromCharString("My Map YY"));


    // hardcoded fill of SUPPORTED LOCATIONS for prototyping
    uint32_t supportedLocationId_A  = 7;
    uint32_t supportedLocationId_B  = 1234567;
    uint32_t supportedLocationId_C  = 10050;
    uint32_t supportedLocationId_D  = 0x88888888;


    // Location A has name, floor number, uses map XX
    mServiceAreaInstance.AddSupportedLocation(  supportedLocationId_A, 
                                                DataModel::Nullable<uint_fast8_t>(supportedMapId_XX),
                                                CharSpan::fromCharString("My Location A"),
                                                DataModel::Nullable<int16_t>(4),
                                                DataModel::Nullable<ServiceArea::AreaTypeTag>(),
                                                DataModel::Nullable<ServiceArea::LandmarkTag>(),
                                                DataModel::Nullable<ServiceArea::PositionTag>(),
                                                DataModel::Nullable<ServiceArea::FloorSurfaceTag>() );


    // Location B has name, uses map XX
    mServiceAreaInstance.AddSupportedLocation(  supportedLocationId_B, 
                                                DataModel::Nullable<uint_fast8_t>(supportedMapId_XX),
                                                CharSpan::fromCharString("My Location B"),
                                                DataModel::Nullable<int16_t>(),
                                                DataModel::Nullable<ServiceArea::AreaTypeTag>(),
                                                DataModel::Nullable<ServiceArea::LandmarkTag>(),
                                                DataModel::Nullable<ServiceArea::PositionTag>(),
                                                DataModel::Nullable<ServiceArea::FloorSurfaceTag>() );




   // Location C has full SemData, no name, Map YY
    mServiceAreaInstance.AddSupportedLocation(  supportedLocationId_C, 
                                                DataModel::Nullable<uint_fast8_t>(supportedMapId_YY),
                                                CharSpan(),
                                                DataModel::Nullable<int16_t>(-1),
                                                DataModel::Nullable<ServiceArea::AreaTypeTag>(ServiceArea::AreaTypeTag::kPlayRoom),
                                                DataModel::Nullable<ServiceArea::LandmarkTag>(ServiceArea::LandmarkTag::kBackDoor),
                                                DataModel::Nullable<ServiceArea::PositionTag>(ServiceArea::PositionTag::kNextTo),
                                                DataModel::Nullable<ServiceArea::FloorSurfaceTag>(ServiceArea::FloorSurfaceTag::kConcrete) );

    // Location D has null values for all HomeLocationStruct fields, Map YY
    mServiceAreaInstance.AddSupportedLocation(   supportedLocationId_D, 
                                                DataModel::Nullable<uint_fast8_t>(supportedMapId_YY),
                                                CharSpan::fromCharString("My Location D"),
                                                DataModel::Nullable<int16_t>(),
                                                DataModel::Nullable<ServiceArea::AreaTypeTag>(),
                                                DataModel::Nullable<ServiceArea::LandmarkTag>(ServiceArea::LandmarkTag::kCouch),
                                                DataModel::Nullable<ServiceArea::PositionTag>(ServiceArea::PositionTag::kNextTo),
                                                DataModel::Nullable<ServiceArea::FloorSurfaceTag>(ServiceArea::FloorSurfaceTag::kHardwood) );


    mServiceAreaInstance.SetCurrentLocation(supportedLocationId_C);
}


void RvcDevice::SetDeviceToIdleState()
{
    if (mCharging)
    {
        mDocked = true;
        mOperationalStateInstance.SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kCharging));
    }
    else if (mDocked)
    {
        mOperationalStateInstance.SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kDocked));
    }
    else
    {
        mOperationalStateInstance.SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kStopped));
    }
}

void RvcDevice::HandleRvcRunChangeToMode(uint8_t newMode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    uint8_t currentState = mOperationalStateInstance.GetCurrentOperationalState();
    uint8_t currentMode  = mRunModeInstance.GetCurrentMode();

    switch (currentState)
    {
    case to_underlying(OperationalState::OperationalStateEnum::kStopped):
    case to_underlying(RvcOperationalState::OperationalStateEnum::kDocked):
    case to_underlying(RvcOperationalState::OperationalStateEnum::kCharging): {
        // We could be in the charging state with an RvcRun mode != idle.
        if (currentMode != RvcRunMode::ModeIdle && newMode != RvcRunMode::ModeIdle)
        {
            response.status = to_underlying(ModeBase::StatusCode::kInvalidInMode);
            response.statusText.SetValue(
                chip::CharSpan::fromCharString("Change to the mapping or cleaning mode is only allowed from idle"));
            return;
        }

        mCharging = false;
        mDocked   = false;
        mRunModeInstance.UpdateCurrentMode(newMode);
        mOperationalStateInstance.SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kRunning));
        response.status = to_underlying(ModeBase::StatusCode::kSuccess);
        return;
    }
    break;
    case to_underlying(OperationalState::OperationalStateEnum::kRunning): {
        if (newMode != RvcRunMode::ModeIdle)
        {
            response.status = to_underlying(ModeBase::StatusCode::kInvalidInMode);
            response.statusText.SetValue(
                chip::CharSpan::fromCharString("Change to the mapping or cleaning mode is only allowed from idle"));
            return;
        }

        mRunModeInstance.UpdateCurrentMode(newMode);
        mOperationalStateInstance.SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger));
        response.status = to_underlying(ModeBase::StatusCode::kSuccess);
        return;
    }
    break;
    }

    // If we fall through at any point, it's because the change is not supported in the current state.
    response.status = to_underlying(ModeBase::StatusCode::kInvalidInMode);
    response.statusText.SetValue(chip::CharSpan::fromCharString("This change is not allowed at this time"));
}

void RvcDevice::HandleRvcCleanChangeToMode(uint8_t newMode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    uint8_t rvcRunCurrentMode = mRunModeInstance.GetCurrentMode();

    if (rvcRunCurrentMode != RvcRunMode::ModeIdle)
    {
        response.status = to_underlying(ModeBase::StatusCode::kInvalidInMode);
        response.statusText.SetValue(chip::CharSpan::fromCharString("Change of the cleaning mode is only allowed in Idle."));
        return;
    }

    response.status = to_underlying(ModeBase::StatusCode::kSuccess);
}

void RvcDevice::HandleOpStatePauseCallback(Clusters::OperationalState::GenericOperationalError & err)
{
    // This method is only called if the device is in a Pause-compatible state, i.e. `Running` or `SeekingCharger`.
    mStateBeforePause = mOperationalStateInstance.GetCurrentOperationalState();
    auto error = mOperationalStateInstance.SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kPaused));
    err.Set((error == CHIP_NO_ERROR) ? to_underlying(OperationalState::ErrorStateEnum::kNoError)
                                     : to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation));
}

void RvcDevice::HandleOpStateResumeCallback(Clusters::OperationalState::GenericOperationalError & err)
{
    uint8_t targetState = to_underlying(OperationalState::OperationalStateEnum::kRunning);

    switch (mOperationalStateInstance.GetCurrentOperationalState())
    {
    case to_underlying(RvcOperationalState::OperationalStateEnum::kCharging):
    case to_underlying(RvcOperationalState::OperationalStateEnum::kDocked): {
        if (mRunModeInstance.GetCurrentMode() != RvcRunMode::ModeCleaning &&
            mRunModeInstance.GetCurrentMode() != RvcRunMode::ModeMapping)
        {
            err.Set(to_underlying(OperationalState::ErrorStateEnum::kCommandInvalidInState));
            return;
        }
    }
    break;
    case to_underlying(OperationalState::OperationalStateEnum::kPaused): {
        if (mStateBeforePause == to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger))
        {
            targetState = to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger);
        }
    }
    break;
    default:
        // This method is only called if the device is in a resume-compatible state, i.e. `Charging`, `Docked` or
        // `Paused`. Therefore, we do not expect to ever enter this branch.
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kCommandInvalidInState));
        return;
    }

    auto error = mOperationalStateInstance.SetOperationalState(targetState);

    err.Set((error == CHIP_NO_ERROR) ? to_underlying(OperationalState::ErrorStateEnum::kNoError)
                                     : to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation));
}

void RvcDevice::HandleOpStateGoHomeCallback(Clusters::OperationalState::GenericOperationalError & err)
{
    switch (mOperationalStateInstance.GetCurrentOperationalState())
    {
    case to_underlying(OperationalState::OperationalStateEnum::kStopped): {
        if (mRunModeInstance.GetCurrentMode() != RvcRunMode::ModeIdle)
        {
            err.Set(to_underlying(OperationalState::ErrorStateEnum::kCommandInvalidInState));
            return;
        }

        auto error = mOperationalStateInstance.SetOperationalState(
            to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger));

        err.Set((error == CHIP_NO_ERROR) ? to_underlying(OperationalState::ErrorStateEnum::kNoError)
                                         : to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation));
    }
    break;
    default:
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kCommandInvalidInState));
        return;
    }
}


bool RvcDevice::HandleIsSetSelectedLocationCallback(char* statusText)
{
    bool canSet;

    switch(mRunModeInstance.GetCurrentMode())
    {
    case RvcRunMode::ModeIdle:
        canSet = true;
        break;

    case RvcRunMode::ModeCleaning:
        canSet = false;
        strncat(statusText, "Cannot select locations while cleaning", ServiceArea::kMaxSizeStatusText);
        break;

    case RvcRunMode::ModeMapping:
        canSet = false;
        strncat(statusText, "Cannot select locations while mapping", ServiceArea::kMaxSizeStatusText);
        break;

    default:
        canSet = false;
        strncat(statusText, "Cannot select locations - unknown mode", ServiceArea::kMaxSizeStatusText);
        break;
    }

    return canSet;
}

void RvcDevice::HandleChargedMessage()
{
    if (mOperationalStateInstance.GetCurrentOperationalState() !=
        to_underlying(RvcOperationalState::OperationalStateEnum::kCharging))
    {
        ChipLogError(NotSpecified, "RVC App: The 'Charged' command is only accepted when the device is in the 'Charging' state.");
        return;
    }

    mCharging = false;

    if (mRunModeInstance.GetCurrentMode() == RvcRunMode::ModeIdle)
    {
        if (mDocked) // assuming that we can't be charging the device while it is not docked.
        {
            mOperationalStateInstance.SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kDocked));
        }
        else
        {
            mOperationalStateInstance.SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kStopped));
        }
    }
    else
    {
        mOperationalStateInstance.SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kRunning));
    }
}

void RvcDevice::HandleChargingMessage()
{
    if (mOperationalStateInstance.GetCurrentOperationalState() != to_underlying(RvcOperationalState::OperationalStateEnum::kDocked))
    {
        ChipLogError(NotSpecified, "RVC App: The 'Charging' command is only accepted when the device is in the 'Docked' state.");
        return;
    }

    mCharging = true;

    mOperationalStateInstance.SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kCharging));
}

void RvcDevice::HandleDockedMessage()
{
    if (mOperationalStateInstance.GetCurrentOperationalState() != to_underlying(OperationalState::OperationalStateEnum::kStopped))
    {
        ChipLogError(NotSpecified, "RVC App: The 'Docked' command is only accepted when the device is in the 'Stopped' state.");
        return;
    }

    mDocked = true;

    mOperationalStateInstance.SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kDocked));
}

void RvcDevice::HandleChargerFoundMessage()
{
    if (mOperationalStateInstance.GetCurrentOperationalState() !=
        to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger))
    {
        ChipLogError(NotSpecified,
                     "RVC App: The 'ChargerFound' command is only accepted when the device is in the 'SeekingCharger' state.");
        return;
    }

    mCharging = true;
    mDocked   = true;

    mOperationalStateInstance.SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kCharging));
}

void RvcDevice::HandleLowChargeMessage()
{
    if (mOperationalStateInstance.GetCurrentOperationalState() != to_underlying(OperationalState::OperationalStateEnum::kRunning))
    {
        ChipLogError(NotSpecified, "RVC App: The 'LowCharge' command is only accepted when the device is in the 'Running' state.");
        return;
    }

    mOperationalStateInstance.SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger));
}

void RvcDevice::HandleActivityCompleteEvent()
{
    if (mOperationalStateInstance.GetCurrentOperationalState() != to_underlying(OperationalState::OperationalStateEnum::kRunning))
    {
        ChipLogError(NotSpecified,
                     "RVC App: The 'ActivityComplete' command is only accepted when the device is in the 'Running' state.");
        return;
    }

    mRunModeInstance.UpdateCurrentMode(RvcRunMode::ModeIdle);

    Optional<DataModel::Nullable<uint32_t>> a(DataModel::Nullable<uint32_t>(100));
    Optional<DataModel::Nullable<uint32_t>> b(DataModel::Nullable<uint32_t>(10));
    mOperationalStateInstance.OnOperationCompletionDetected(0, a, b);

    mOperationalStateInstance.SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger));
}

void RvcDevice::HandleErrorEvent(const std::string & error)
{
    detail::Structs::ErrorStateStruct::Type err;

    if (error == "UnableToStartOrResume")
    {
        err.errorStateID = to_underlying(OperationalState::ErrorStateEnum::kUnableToStartOrResume);
    }
    else if (error == "UnableToCompleteOperation")
    {
        err.errorStateID = to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation);
    }
    else if (error == "CommandInvalidInState")
    {
        err.errorStateID = to_underlying(OperationalState::ErrorStateEnum::kCommandInvalidInState);
    }
    else if (error == "FailedToFindChargingDock")
    {
        err.errorStateID = to_underlying(RvcOperationalState::ErrorStateEnum::kFailedToFindChargingDock);
    }
    else if (error == "Stuck")
    {
        err.errorStateID = to_underlying(RvcOperationalState::ErrorStateEnum::kStuck);
    }
    else if (error == "DustBinMissing")
    {
        err.errorStateID = to_underlying(RvcOperationalState::ErrorStateEnum::kDustBinMissing);
    }
    else if (error == "DustBinFull")
    {
        err.errorStateID = to_underlying(RvcOperationalState::ErrorStateEnum::kDustBinFull);
    }
    else if (error == "WaterTankEmpty")
    {
        err.errorStateID = to_underlying(RvcOperationalState::ErrorStateEnum::kWaterTankEmpty);
    }
    else if (error == "WaterTankMissing")
    {
        err.errorStateID = to_underlying(RvcOperationalState::ErrorStateEnum::kWaterTankMissing);
    }
    else if (error == "WaterTankLidOpen")
    {
        err.errorStateID = to_underlying(RvcOperationalState::ErrorStateEnum::kWaterTankLidOpen);
    }
    else if (error == "MopCleaningPadMissing")
    {
        err.errorStateID = to_underlying(RvcOperationalState::ErrorStateEnum::kMopCleaningPadMissing);
    }
    else
    {
        ChipLogError(NotSpecified, "Unhandled command: The 'Error' key of the 'ErrorEvent' message is not valid.");
        return;
    }

    mOperationalStateInstance.OnOperationalErrorDetected(err);
}

void RvcDevice::HandleClearErrorMessage()
{
    if (mOperationalStateInstance.GetCurrentOperationalState() != to_underlying(OperationalState::OperationalStateEnum::kError))
    {
        ChipLogError(NotSpecified, "RVC App: The 'ClearError' command is only excepted when the device is in the 'Error' state.");
        return;
    }

    mRunModeInstance.UpdateCurrentMode(RvcRunMode::ModeIdle);
    SetDeviceToIdleState();
}

void RvcDevice::HandleResetMessage()
{
    mRunModeInstance.UpdateCurrentMode(RvcRunMode::ModeIdle);
    mOperationalStateInstance.SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kStopped));
    mCleanModeInstance.UpdateCurrentMode(RvcCleanMode::ModeQuick);
}


// *****************************************************************************
//   RVC SERVICE AREA DELEGATE IMPLEMENTATION  

using namespace chip::app::Clusters::ServiceArea;



//*************************************************************************
// command support

bool RvcDevice::IsSetSelectedLocationsAllowed(char* statusText)
{
    return true; // TODO IMPLEMENT
};

bool RvcDevice::HandleSetSelectLocations(const Commands::SelectLocations::DecodableType & req, 
                        SelectLocationsStatus & locationStatus, char* statusText, bool & useStatusText) 
{
    bool ret_value = false;

    ret_value = true; // TODO IMPLEMENT

    return ret_value;
};

bool RvcDevice::HandleSkipCurrentLocation(char* skipStatusText)
{
    bool ret_value = false;

    ret_value = true; // TODO IMPLEMENT

    return ret_value;
};


//*************************************************************************
// Supported Locations accessors

bool RvcDevice::IsSupportedLocationsChangeAllowed()
{
    return true; // TODO
}

uint32_t RvcDevice::GetNumberOfSupportedLocations() 
{
    return mSupportedLocations.size();
}


bool RvcDevice::GetSupportedLocationByIndex(uint32_t listIndex, LocationStructureWrapper & aSupportedLocation) 
{
    bool ret_value = false;

    if (listIndex < mSupportedLocations.size())
    {
        aSupportedLocation = mSupportedLocations[listIndex];
        ret_value = true;
    }

    return ret_value;
};


bool RvcDevice::GetSupportedLocationById(uint32_t aLocationId, uint32_t & listIndex, LocationStructureWrapper & aSupportedLocation)
{
    // optional optimization of base delegate function
    // still using linear search, 
    // but don't need to copy every location (because direct access to list is available)
    bool ret_value = false;

    listIndex = 0;
    uint32_t listSize = mSupportedLocations.size();

    while (listIndex < listSize)
    {
        if (mSupportedLocations[listIndex].locationID == aLocationId)
        {
            aSupportedLocation = mSupportedLocations[listIndex];
            ret_value = true;
            break;
        }

        ++listIndex;
    }

    return ret_value;
};


bool RvcDevice::AddSupportedLocation(const LocationStructureWrapper & newLocation, uint32_t & listIndex)
{
    bool ret_value = false;

    // server instance (caller)  is responsible for ensuring no duplicate locationsIds, list size not exceeded, etc.

    // double check list size to ensure no memory issues
    if (mSupportedLocations.size() < kMaxNumSupportedLocations)
    {
        // not sorting list, number of locations normally expected to be small, max 255
        mSupportedLocations.push_back(newLocation);
        listIndex = mSupportedMaps.size() - 1; // new element is last in list
        ret_value = true;
    }
    else
    {
        ChipLogError(Zcl,  "AddSupportedLocation %u - supported locations list is already at maximum size %u",
                            newLocation.locationID, (uint32_t)kMaxNumSupportedLocations);
    }

    return ret_value;
}


bool RvcDevice::ModifySupportedLocation(uint32_t listIndex, const LocationStructureWrapper & modifiedLocation)
{
    bool ret_value = false;

    // server instance (caller) is responsible for ensuring no duplicate locationsIds, list size not exceeded, etc.

    // double check that locationID's match
    VerifyOrExit((modifiedLocation.locationID == mSupportedLocations[listIndex].locationID), 
        ChipLogError(Zcl,  "ModifySupportedLocation - new locationID %u does not match existing locationID %u", 
                        modifiedLocation.locationID, mSupportedLocations[listIndex].locationID));   


    // checks passed, update the attribute
    mSupportedLocations[listIndex] = modifiedLocation;
    ret_value = true;

exit:

    return ret_value;
}


bool RvcDevice::ClearSupportedLocations()
{
    bool ret_value = false;

    if (mSupportedLocations.size() > 0)
    {
        mSupportedLocations.clear();
        ret_value = true;
    }

    return ret_value;
}


//*************************************************************************
// Supported Maps accessors

bool RvcDevice::IsSupportedMapChangeAllowed()
{
    return true; // TODO
}

uint32_t RvcDevice::GetNumberOfSupportedMaps() 
{
    return mSupportedMaps.size();
}

bool RvcDevice::GetSupportedMapByIndex(uint32_t listIndex, MapStructureWrapper & aSupportedMap) 
{
    bool ret_value = false;

    if (listIndex < mSupportedMaps.size())
    {
        aSupportedMap = mSupportedMaps[listIndex];
        ret_value = true;
    }

    return ret_value;
};


bool RvcDevice::GetSupportedMapById(uint8_t aMapId, uint32_t & listIndex, MapStructureWrapper & aSupportedMap)
{
    // optional optimization of base delegate function
    // still using linear search, 
    // but don't need to copy every map (because direct access to list is available)
    bool ret_value = false;

    listIndex = 0;
    uint32_t listSize = mSupportedMaps.size();

    while (listIndex < listSize)
    {
        if (mSupportedMaps[listIndex].mapID == aMapId)
        {
            aSupportedMap = mSupportedMaps[listIndex];
            ret_value = true;
            break;
        }

        ++listIndex;
    }

    return ret_value;
};


bool RvcDevice::AddSupportedMap(const MapStructureWrapper & newMap, uint32_t & listIndex)
{
    bool ret_value = false;

    // server instance class is responsible for ensuring no duplicate locationsIds, list size not exceeded, etc.

    // double check list size to ensure no memory issues
    if (mSupportedMaps.size() < kMaxNumSupportedMaps)
    {
        // not sorting list, number of locations normally expected to be small, max 255
        mSupportedMaps.push_back(newMap);
        listIndex = mSupportedMaps.size() - 1; // new element is last in list
    }
    else
    {
        ChipLogError(Zcl,  "AddSupportedMap %u - supported maps list is already at maximum size %u",
                                newMap.mapID, (uint32_t)kMaxNumSupportedMaps);
    }

    return ret_value;
}


bool RvcDevice::ModifySupportedMap(uint32_t listIndex, const MapStructureWrapper & modifiedMap)
{
    bool ret_value = false;

    // server instance (caller) is responsible for ensuring no duplicate locationsIds, list size not exceeded, etc.

    // double check that mapID's match
    VerifyOrExit((modifiedMap.mapID == mSupportedMaps[listIndex].mapID),
        ChipLogError(Zcl,  "ModifySupportedMap - mapID's do not match, new lmapID %u, existing mapID %u", 
                        modifiedMap.mapID, mSupportedMaps[listIndex].mapID));   

    // save modified map
    mSupportedMaps[listIndex] = modifiedMap;
    ret_value = true;

exit:
    return ret_value;
}


bool RvcDevice::ClearSupportedMaps()
{
    bool ret_value = false;

    if (mSupportedMaps.size() > 0)
    {
        mSupportedMaps.clear();
        ret_value = true;
    }

    return ret_value;
}


//*************************************************************************
// Selected Locations accessors

uint32_t RvcDevice::GetNumberOfSelectedLocations()
{
    return mSelectedLocations.size();
}


bool RvcDevice::GetSelectedLocationByIndex(uint32_t listIndex, uint32_t & aSelectedLocation)
{
    bool ret_value = false;

    if (listIndex < mSelectedLocations.size())
    {
        aSelectedLocation = mSelectedLocations[listIndex];
        ret_value = true;
    }

    return ret_value;
};


bool RvcDevice::AddSelectedLocation(uint32_t aLocationId, uint32_t & listIndex)
{
    bool ret_value = false;

    // server instance class is responsible for ensuring no duplicate locationsIds, list size not exceeded, etc.

    // double check list size to ensure no memory issues
    if (mSelectedLocations.size() < kMaxNumSelectedLocations)
    {
        // not sorting list, number of locations normally expected to be small, max 255
        mSelectedLocations.push_back(aLocationId);
        listIndex = mSelectedLocations.size() - 1; // new element is last in list
    }
    else
    {
        ChipLogError(Zcl,  "AddSelectedLocation %u - selected locations list is already at maximum size %u",
                                aLocationId, (uint32_t)kMaxNumSelectedLocations);
    }

    return ret_value;
}


bool RvcDevice::ClearSelectedLocations()
{
    bool ret_value = false;

    if (mSelectedLocations.size() > 0)
    {
        mSelectedLocations.clear();
        ret_value = true;
    }

    return ret_value;
}


//*************************************************************************
// Progress List accessors

uint32_t RvcDevice::GetNumberOfProgressElements() 
{
    return mProgressList.size();
}


bool RvcDevice::GetProgressElementByIndex(uint32_t listIndex,  Structs::ProgressStruct::Type & aProgressElement) 
{
    bool ret_value = false;

    if (listIndex < mProgressList.size())
    {
        aProgressElement = mProgressList[listIndex];
        ret_value = true;
    }

    return ret_value;
};


bool RvcDevice::GetProgressElementById(uint32_t aLocationId, uint32_t & listIndex, Structs::ProgressStruct::Type & aProgressElement)
{
    // optional optimization of base delegate function
    // still using linear search, 
    // but don't need to copy every map (because direct access to list is available)
    bool ret_value = false;

    listIndex = 0;
    uint32_t listSize = mProgressList.size();

    while (listIndex < listSize)
    {
        if (mProgressList[listIndex].locationID == aLocationId)
        {
            aProgressElement = mProgressList[listIndex];
            ret_value = true;
            break;
        }

        ++listIndex;
    }

    return ret_value;
};


bool RvcDevice::AddProgressElement(const Structs::ProgressStruct::Type & newProgressElement, uint32_t & listIndex)
{
    bool ret_value = false;

    // server instance class is responsible for ensuring no duplicate locationsIds, list size not exceeded, etc.

    // double check list size to ensure no memory issues
    if (mProgressList.size() < kMaxNumProgressElements)
    {
        // not sorting list, number of locations normally expected to be small, max 255
        mProgressList.push_back(newProgressElement);
        listIndex = mProgressList.size() - 1; // new element is last in list
    }
    else
    {
        ChipLogError(Zcl,  "AddProgressElement %u -progress list is already at maximum size %u", 
                            newProgressElement.locationID, (uint32_t)kMaxNumProgressElements);
    }

    return ret_value;
}


bool RvcDevice::ModifyProgressElement(uint32_t listIndex, const Structs::ProgressStruct::Type & modifiedProgressElement) 
{
    bool ret_value = false;

    // TODO

    return ret_value; 
}


bool RvcDevice::ClearProgress()
{
    bool ret_value = false;

    if (mProgressList.size() > 0)
    {
        mProgressList.clear();
        ret_value = true;
    }

    return ret_value;
}
