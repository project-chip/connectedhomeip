#pragma once

#include "rvc-mode-delegates.h"
#include "rvc-operational-state-delegate.h"
#include <app/clusters/mode-base-server/mode-base-server.h>
#include <app/clusters/operational-state-server/operational-state-server.h>
#include <app/clusters/service-area-server/service-area-server.h>
#include <app/clusters/service-area-server/service-area-delegate.h>

#include <string>

namespace chip {
namespace app {
namespace Clusters {

class RvcDevice :  public ServiceArea::Delegate
{
private:
    RvcRunMode::RvcRunModeDelegate mRunModeDelegate;
    ModeBase::Instance mRunModeInstance;

    RvcCleanMode::RvcCleanModeDelegate mCleanModeDelegate;
    ModeBase::Instance mCleanModeInstance;

    RvcOperationalState::RvcOperationalStateDelegate mOperationalStateDelegate;
    RvcOperationalState::Instance mOperationalStateInstance;

    // note: inheritance from RvcServiceAreaDelegate vs declaring an object eliminates a number of callbacks
    // there is only one associated ServiceArea delegate, so there should be no confusion.
    ServiceArea::Instance mServiceAreaInstance;

    // containers for service area array attributes
    std::vector<ServiceArea::LocationStructureWrapper>       mSupportedLocations;
    std::vector<ServiceArea::MapStructureWrapper>            mSupportedMaps;
    std::vector<uint32_t>                                    mSelectedLocations;
    std::vector<ServiceArea::Structs::ProgressStruct::Type>  mProgressList;

    bool mDocked   = false;
    bool mCharging = false;

    uint8_t mStateBeforePause = 0;

public:
    /**
     * This class is responsible for initialising all the RVC clusters and manging the interactions between them as required by
     * the specific "business logic". See the state machine diagram.
     * @param aRvcClustersEndpoint The endpoint ID where all the RVC clusters exist.
     */
    explicit RvcDevice(EndpointId aRvcClustersEndpoint) :
        mRunModeDelegate(), mRunModeInstance(&mRunModeDelegate, aRvcClustersEndpoint, RvcRunMode::Id, 0), mCleanModeDelegate(),
        mCleanModeInstance(&mCleanModeDelegate, aRvcClustersEndpoint, RvcCleanMode::Id, 0), mOperationalStateDelegate(),
        mOperationalStateInstance(&mOperationalStateDelegate, aRvcClustersEndpoint),
        mServiceAreaInstance(this, aRvcClustersEndpoint, BitMask<ServiceArea::Feature>(0))
    {
        // set the current-mode at start-up
        mRunModeInstance.UpdateCurrentMode(RvcRunMode::ModeIdle);

        // Hypothetically, the device checks if it is physically docked or charging
        SetDeviceToIdleState();

        // set callback functions
        mRunModeDelegate.SetHandleChangeToMode(&RvcDevice::HandleRvcRunChangeToMode, this);
        mCleanModeDelegate.SetHandleChangeToMode(&RvcDevice::HandleRvcCleanChangeToMode, this);
        mOperationalStateDelegate.SetPauseCallback(&RvcDevice::HandleOpStatePauseCallback, this);
        mOperationalStateDelegate.SetResumeCallback(&RvcDevice::HandleOpStateResumeCallback, this);
        mOperationalStateDelegate.SetGoHomeCallback(&RvcDevice::HandleOpStateGoHomeCallback, this);
    }

    /**
     * Init all the clusters used by this device.
     */
    void Init();

    /**
     * Sets the device to an idle state, that is either the STOPPED, DOCKED or CHARGING state, depending on physical information.
     * Note: in this example this is based on the mDocked and mChanging boolean variables.
     */
    void SetDeviceToIdleState();

    /**
     * Handles the RvcRunMode command requesting a mode change.
     */
    void HandleRvcRunChangeToMode(uint8_t newMode, ModeBase::Commands::ChangeToModeResponse::Type & response);

    /**
     * Handles the RvcCleanMode command requesting a mode change.
     */
    void HandleRvcCleanChangeToMode(uint8_t newMode, ModeBase::Commands::ChangeToModeResponse::Type & response);

    /**
     * Handles the RvcOperationalState pause command.
     */
    void HandleOpStatePauseCallback(Clusters::OperationalState::GenericOperationalError & err);

    /**
     * Handles the RvcOperationalState resume command.
     */
    void HandleOpStateResumeCallback(Clusters::OperationalState::GenericOperationalError & err);

    /**
     * Handles the RvcOperationalState GoHome command.
     */
    void HandleOpStateGoHomeCallback(Clusters::OperationalState::GenericOperationalError & err);

    /**
     * @brief Handles the check to see if the Selected Locations can be changed
     * @param locationText if failure description of mode or condition prohibiting change
     *        size kMaxSizeStatusText + 1 for termination char
     * @return true if Selected Locations can be changed
    */
    bool HandleIsSetSelectedLocationCallback(char* locationText);

    /**
     * Updates the state machine when the device becomes fully-charged.
     */
    void HandleChargedMessage();

    void HandleChargingMessage();

    void HandleDockedMessage();

    void HandleChargerFoundMessage();

    void HandleLowChargeMessage();

    void HandleActivityCompleteEvent();

    /**
     * Sets the device to an error state with the error state ID matching the error name given.
     * @param error The error name. Could be one of UnableToStartOrResume, UnableToCompleteOperation, CommandInvalidInState,
     * FailedToFindChargingDock, Stuck, DustBinMissing, DustBinFull, WaterTankEmpty, WaterTankMissing, WaterTankLidOpen or
     * MopCleaningPadMissing.
     */
    void HandleErrorEvent(const std::string & error);

    void HandleClearErrorMessage();

    void HandleResetMessage();


    //*************************************************************************
    // RVC Service Area overrides

    // command support

    bool IsSetSelectedLocationAllowed(char* statusText) override;

    bool HandleSetSelectLocations(const ServiceArea::Commands::SelectLocations::DecodableType & req, 
                        ServiceArea::SelectLocationsStatus & locationStatus, char* statusText, bool & useStatusText) override;

    bool HandleSkipCurrentLocation(char* skipStatusText) override;

    //*************************************************************************
    // Supported Locations accessors

    bool IsSupportedLocationChangeAllowed();

    uint32_t GetNumberOfSupportedLocations() override;

    bool GetSupportedLocationByIndex(uint32_t listIndex, ServiceArea::LocationStructureWrapper & supportedLocation) override;

    bool GetSupportedLocationById(uint32_t aLocationId, uint32_t & listIndex, ServiceArea::LocationStructureWrapper & supportedLocation) override;

    bool AddSupportedLocation(const ServiceArea::LocationStructureWrapper & newLocation, uint32_t & listIndex) override;

    bool ModifySupportedLocation(uint32_t listIndex, const ServiceArea::LocationStructureWrapper & modifiedLocation) override;

    bool ClearSupportedLocations() override;

    //*************************************************************************
    // Supported Maps accessors

    bool IsSupportedMapChangeAllowed();

    uint32_t GetNumberOfSupportedMaps() override;

    bool GetSupportedMapByIndex(uint32_t listIndex, ServiceArea::MapStructureWrapper & supportedMap) override;

    bool GetSupportedMapById(uint8_t aMapId, uint32_t & listIndex, ServiceArea::MapStructureWrapper & supportedMap) override;

    bool AddSupportedMap(const ServiceArea::MapStructureWrapper & newMap, uint32_t & listIndex) override;

    bool ModifySupportedMap(uint32_t listIndex, const ServiceArea::MapStructureWrapper & newMap) override;

    bool ClearSupportedMaps() override;

    //*************************************************************************
    // Selected Locations accessors

    uint32_t GetNumberOfSelectedLocations() override;

    bool GetSelectedLocationByIndex(uint32_t listIndex, uint32_t & selectedLocation) override;

    // IsSelectedLocation() no override

    bool AddSelectedLocation(uint32_t aLocationId, uint32_t & listIndex) override;

    bool ClearSelectedLocations() override;

    //*************************************************************************
    // Progress accessors

    uint32_t GetNumberOfProgressElements() override;

    bool GetProgressElementByIndex(uint32_t listIndex, ServiceArea::Structs::ProgressStruct::Type & aProgressElement) override;

    bool GetProgressElementById(uint32_t aLocationId, uint32_t & listIndex, ServiceArea::Structs::ProgressStruct::Type & aProgressElement) override;

    bool AddProgressElement(const ServiceArea::Structs::ProgressStruct::Type & newProgressElement, uint32_t & listIndex) override;

    bool ModifyProgressElement(uint32_t listIndex, const ServiceArea::Structs::ProgressStruct::Type & modifiedProgressElement) override;

    bool ClearProgress() override;
};

} // namespace Clusters
} // namespace app
} // namespace chip
