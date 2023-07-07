/*
 *
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

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>

namespace chip {
namespace app {
namespace Clusters {
namespace FanControl {

class Delegate;

class Instance : public AttributeAccessInterface, public CommandHandlerInterface
{
public:
    /**
     * Creates a FanCOntrol cluster instance. The Init() function needs to be called for this instance to be registered and
     * called by the interaction model at the appropriate times.
     * @param aEndpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aFanModeSequence The FanModeSequence attribute value, this is fixed in the cluster.
     * @param aFeature The bitmask value that identifies which features are supported by this instance.
     */
    Instance(EndpointId aEndpointId, FanModeSequenceEnum aFanModeSequence, uint32_t aFeature, Delegate * aDelegate);

    /**
     * Creates a FanCOntrol cluster instance. The Init() function needs to be called for this instance to be registered and
     * called by the interaction model at the appropriate times.
     * @param aEndpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aFanModeSequence The FanModeSequence attribute value, this is fixed in the cluster.
     * @param aSpeedMax The SpeedMax attribute value, this is fixed in the cluster.
     * @param aFeature The bitmask value that identifies which features are supported by this instance.
     */
    Instance(EndpointId aEndpointId, FanModeSequenceEnum aFanModeSequence, uint8_t aSpeedMax, uint32_t aFeature,
             Delegate * aDelegate);

    /**
     * Creates a FanCOntrol cluster instance. The Init() function needs to be called for this instance to be registered and
     * called by the interaction model at the appropriate times.
     * @param aEndpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aFanModeSequence The FanModeSequence attribute value, this is fixed in the cluster.
     * @param aSpeedMax The SpeedMax attribute value, this is fixed in the cluster.
     * @param aRockSupport The RockerSupport attribute value, this is fixed in the cluster. If unused, set to 0.
     * @param aWindSupport The WindSupport attribute value, this is fixed in the cluster. If unused, set to 0.
     * @param aFeature The bitmask value that identifies which features are supported by this instance.
     */
    Instance(EndpointId aEndpointId, FanModeSequenceEnum aFanModeSequence, uint8_t aSpeedMax, RockBitmap aRockSupport,
             WindBitmap aWindSupport, uint32_t aFeature, Delegate * aDelegate);

    /**
     * Initialise the FanControl server instance.
     * @return Returns an error if the given endpoint and cluster ID have not been enabled in zap,
     * if the CommandHandler or AttributeHandler registration fails or if the AppInit() returns an error.
     */
    CHIP_ERROR Init();

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & ctx) override;
    CHIP_ERROR EnumerateAcceptedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context) override;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

    template <typename RequestT, typename FuncT>
    void HandleCommand(HandlerContext & handlerContext, FuncT func);

    // Attribute setters
    Status UpdateFanMode(FanModeEnum fanMode);
    Status UpdatePercentSetting(DataModel::Nullable<Percent> percentSetting);
    Status UpdatePercentCurrent(Percent percentCurrent);
    Status UpdateSpeedSetting(DataModel::Nullable<uint8_t> speedSetting);
    Status UpdateSpeedCurrent(uint8_t speedCurrent);
    Status UpdateRockSetting(RockBitmap rockSetting);
    Status UpdateWindSetting(WindBitmap windSetting);
    Status UpdateAirflowDirection(AirflowDirectionEnum airflowDirection);
    Status UpdatePercentageAndSpeedSetting(DataModel::Nullable<Percent> percentSetting);
    Status UpdatePercentageAndSpeedSetting(DataModel::Nullable<uint8_t> speedSetting);
    Status UpdatePercentageAndSpeedCurrent(Percent percentCurrent);
    Status UpdatePercentageAndSpeedCurrent(uint8_t speedCurrent);

    // Attribute getters
    FanModeEnum GetFanMode() const { return mFanMode; }
    FanModeSequenceEnum GetFanModeSequence() const { return mFanModeSequence; }
    DataModel::Nullable<Percent> GetPercentSetting() const { return mPercentSetting; }
    Percent GetPercentCurrent() const { return mPercentCurrent; }
    uint8_t GetSpeedMax() const { return mSpeedMax; }
    DataModel::Nullable<uint8_t> GetSpeedSetting() const { return mSpeedSetting; }
    uint8_t GetSpeedCurrent() const { return mSpeedCurrent; }
    RockBitmap GetRockSupport() const { return mRockSupport; }
    RockBitmap GetRockSetting() const { return mRockSetting; }
    WindBitmap GetWindSupport() const { return mWindSupport; }
    WindBitmap GetWindSetting() const { return mWindSetting; }
    AirflowDirectionEnum GetAirflowDirection() const { return mAirflowDirection; }
    EndpointId GetEndpointId() const { return mEndpointId; }

    /**
     * Check if the MultiSpeed feature is supported.
     */
    bool SupportsMultiSpeed() const { return hasFeature(Feature::kMultiSpeed); }

    /**
     * Check if the Auto feature is supported.
     */
    bool SupportsAuto() const { return hasFeature(Feature::kAuto); }

    /**
     * Check if the Rocking feature is supported.
     */
    bool SupportsRocking() const { return hasFeature(Feature::kRocking); }

    /**
     * Check if the Wind feature is supported.
     */
    bool SupportsWind() const { return hasFeature(Feature::kWind); }

    /**
     * Check if the Step feature is supported.
     */
    bool SupportsStep() const { return hasFeature(Feature::kStep); }

    /**
     * Check if the Airflow Direction feature is supported.
     */
    bool SupportsAirflowDirection() const { return hasFeature(Feature::kAirflowDirection); }

private:
    static const mPercentMax = 100;

    EndpointId mEndpointId{};

    // Attribute data store
    FanModeEnum mFanMode = FanModeEnum::kOff;
    FanModeSequenceEnum mFanModeSequence;
    DataModel::Nullable<Percent> mPercentSetting = 0;
    Percent mPercentCurrent                      = 0;
    uint8_t mSpeedMax                            = 0;
    DataModel::Nullable<uint8_t> mSpeedSetting   = 0;
    uint8_t mSpeedCurrent                        = 0;
    RockBitmap mRockSupport                      = 0;
    RockBitmap mRockSetting                      = 0;
    WindBitmap mWindSupport                      = 0;
    WindBitmap mWindSetting                      = 0;
    AirflowDirectionEnum mAirflowDirection       = AirflowDirectionEnum::kForward;
    uint32_t mFeatureMap                         = 0;

    Delegate * mDelegate = nullptr;

    /**
     * Internal feature checking function.
     */
    bool hasFeature(Feature feature) const { return ((mFeatureMap & to_underlying(feature)) != 0); }

    /**
     * Internal function to nullify the PercentSetting attribute.
     */
    Status nullifyPercentSetting();

    /**
     * Internal function to nullify the SpeedSetting attribute.
     */
    Status nullifySpeedSetting();

    /**
     * Internal fan mode write handler function.
     */
    Status handleFanModeWrite(AttributeValueDecoder & aDecoder);

    /**
     * Internal percent setting write handler function.
     */
    Status handlePercentSettingWrite(AttributeValueDecoder & aDecoder);

    /**
     * Internal speed setting write handler function.
     */
    Status handleSpeedSettingWrite(AttributeValueDecoder & aDecoder);

    /**
     * Internal rock setting write handler function.
     */
    Status handleRockSettingWrite(AttributeValueDecoder & aDecoder);

    /**
     * Internal wind setting write handler function.
     */
    Status handleWindSettingWrite(AttributeValueDecoder & aDecoder);

    /**
     * Internal airflow direction write handler function.
     */
    Status handleAirflowDirectionWrite(AttributeValueDecoder & aDecoder);

    /**
     * Internal step command handler function.
     */
    void handleStep(HandlerContext & ctx, const Commands::ChangeToMode::DecodableType & commandData);

    /**
     * Internal function to convert percentage into speed
     */
    static uint8_t getSpeedFromPercent(float percent, uint8_t speedMax);

    /**
     * Internal function to convert speed into percentage
     */
    static Percent getPercentFromSpeed(uint8_t speed);
};

class Delegate
{
public:
    /**
     * This init function will be called during the FanControl server initialization after the Instance information has been
     * validated and the Instance has been registered. This can be used to initialise app logic if required.
     */
    virtual CHIP_ERROR Init() { return CHIP_NO_ERROR; }

    /**
     * This function will be called when the FanMode attribute is changed
     * @param newMode The new FanMode value.
     * @return Returns a CHIP_NO_ERROR if there was no error.
     */
    virtual CHIP_ERROR HandleFanModeChange(FanModeEnum newMode) = 0;

    /**
     * This function will be called when the FanMode attribute is changed to Off. The server will change the SpeedSetting and
     * PercentSetting in this case so are included in the callback.
     * @param newPercentSetting The new PercentSetting value.
     * @param newSpeedSetting The new SpeedSetting value (can be ignored if the feature is not supported)
     * @return Returns a CHIP_NO_ERROR if there was no error.
     *
     */
    virtual CHIP_ERROR HandleFanModeOff(DataModel::Nullable<Percent> newPercentSetting,
                                        DataModel::Nullable<uint8_t> newSpeedSetting) = 0;

    /**
     * This function will be called when the FanMode attribute is changed to Auto. The server will change the SpeedSetting and
     * PercentSetting in this case so are included in the callback.
     * @param newPercentSetting The new PercentSetting value.
     * @param newSpeedSetting The new SpeedSetting value (can be ignored if the feature is not supported)
     * @return Returns a CHIP_NO_ERROR if there was no error.
     */
    virtual CHIP_ERROR HandleFanModeAuto(DataModel::Nullable<Percent> newPercentSetting,
                                         DataModel::Nullable<uint8_t> newSpeedSetting)
    {
        return CHIP_NO_ERROR;
    }

    /**
     * This function will be called when the PercentSetting or SpeedSetting attributes are changed.
     * @param newPercentSetting The new PercentSetting value.
     * @param newSpeedSetting The new SpeedSetting value (can be ignored if the feature is not supported)
     * @return Returns a CHIP_NO_ERROR if there was no error.
     */
    virtual CHIP_ERROR HandlePercentSpeedSettingChange(DataModel::Nullable<Percent> newPercentSetting,
                                                       DataModel::Nullable<uint8_t> newSpeedSetting) = 0;

    /**
     * This function will be called when the RockSetting attribute is changed.
     * @param newRockSetting The new RockSetting value.
     * @return Returns a CHIP_NO_ERROR if there was no error.
     */
    virtual CHIP_ERROR HandleRockSettingChange(RockBitmap newRockSetting) { return CHIP_NO_ERROR; }

    /**
     * This function will be called when the WindSetting attribute is changed.
     * @param newWindSetting The new WindSetting value.
     * @return Returns a CHIP_NO_ERROR if there was no error.
     */
    virtual CHIP_ERROR HandleWindSettingChange(WindBitmap newWindSetting) { return CHIP_NO_ERROR; }

    /**
     * This function will be called when the AirflowDirection attribute is changed.
     * @param newAirflowDirection The new AirflowDirection value.
     * @return Returns a CHIP_NO_ERROR if there was no error.
     */
    virtual CHIP_ERROR HandleAirflowDirectionChange(AirflowDirectionEnum newAirflowDirection) { return CHIP_NO_ERROR; }

    /**
     * This function will be called when the Step Command is received and the STEP feature is enabled in the Feature Map
     * @param stepDirection The direction of the step command.
     * @param wrap Whether the step should wrap around to the other end of the sequence.
     * @param lowestOff Whether the lowest setting should be treated as off.
     * @return Returns an interation model status code indicating the result of the command.
     */
    virtual Status HandleStep(StepDirectionEnum stepDirection, bool wrap, bool lowestOff) { return Status::UnsupportedCommand; }

    /**
     * This function will link the delegate to a parent FanControl server instance
     */
    void SetParent(Instance * parent) { this->parent = parent; }

protected:
    // Cluster Attribute Accessors
    FanModeEnum GetClusterFanMode() const { return parent->GetFanMode(); }
    FanModeSequenceEnum GetClusterFanModeSequence() const { return parent->GetFanModeSequence(); }
    DataModel::Nullable<Percent> GetClusterPercentSetting() const { return parent->GetPercentSetting(); }
    Percent GetClusterPercentCurrent() const { return parent->GetPercentCurrent(); }
    uint8_t GetClusterSpeedMax() const { return parent->GetSpeedMax(); }
    DataModel::Nullable<uint8_t> GetClusterSpeedSetting() const { return parent->GetSpeedSetting(); }
    uint8_t GetClusterSpeedCurrent() const { return parent->GetSpeedCurrent(); }
    RockBitmap GetClusterRockSupport() const { return parent->GetRockSupport(); }
    RockBitmap GetClusterRockSetting() const { return parent->GetRockSetting(); }
    WindBitmap GetClusterWindSupport() const { return parent->GetWindSupport(); }
    WindBitmap GetClusterWindSetting() const { return parent->GetWindSetting(); }
    AirflowDirectionEnum GetClusterAirflowDirection() const { return parent->GetAirflowDirection(); }
    EndpointId GetClusterEndpointId() const { return parent->GetEndpointId(); }

    // Cluster Attribute Setters
    Status UpdateClusterFanMode(FanModeEnum fanMode) { return parent->UpdateFanMode(fanMode); }
    Status UpdateClusterPercentSetting(DataModel::Nullable<Percent> percentSetting)
    {
        return parent->UpdatePercentSetting(percentSetting);
    }
    Status UpdateClusterPercentCurrent(Percent percentCurrent) { return parent->UpdatePercentCurrent(percentCurrent); }
    Status UpdateClusterSpeedSetting(uint8_t speedSetting) { return parent->UpdateSpeedSetting(speedSetting); }
    Status UpdateClusterSpeedCurrent(uint8_t speedCurrent) { return parent->UpdateSpeedCurrent(speedCurrent); }
    Status UpdateClusterRockSetting(RockBitmap rockSetting) { return parent->UpdateRockSetting(rockSetting); }
    Status UpdateClusterWindSetting(WindBitmap windSetting) { return parent->UpdateWindSetting(windSetting); }
    Status UpdateClusterAirflowDirection(AirflowDirectionEnum airflowDirection)
    {
        return parent->UpdateAirflowDirection(airflowDirection);
    }
    Status UpdateClusterPercentageAndSpeedSetting(DataModel::Nullable<Percent> percentSetting)
    {
        return parent->UpdatePercentageAndSpeedSetting(percentSetting);
    }
    Status UpdateClusterPercentageAndSpeedSetting(DataModel::Nullable<uint8_t> speedSetting)
    {
        return parent->UpdatePercentageAndSpeedSetting(speedSetting);
    }
    Status UpdateClusterPercentageAndSpeedCurrent(Percent percentCurrent)
    {
        return parent->UpdatePercentageAndSpeedCurrent(percentCurrent);
    }
    Status UpdateClusterPercentageAndSpeedCurrent(uint8_t speedCurrent)
    {
        return parent->UpdatePercentageAndSpeedCurrent(speedCurrent);
    }

private:
    Instance * parent;
}

} // namespace FanControl
} // namespace Clusters
} // namespace app
} // namespace chip