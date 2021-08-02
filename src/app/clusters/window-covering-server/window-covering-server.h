/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <app/common/gen/attribute-id.h>
#include <app/common/gen/enums.h>
#include <app/util/af-types.h>
#include <map>

class WindowCover
{
public:
    enum class Feature
    {
        Lift          = 0x01,
        Tilt          = 0x02,
        PositionAware = 0x04
    };

    struct Mode
    {
        uint8_t motorDirReversed : 1; // bit 0
        uint8_t calibrationMode : 1;  // bit 1
        uint8_t maintenanceMode : 1;  // bit 2
        uint8_t ledDisplay : 1;       // bit 3
    };
    static_assert(sizeof(Mode) == sizeof(uint8_t), "Mode Size is not correct");

    struct ConfigStatus
    {
        uint8_t operational : 1;             // bit 0 M
        uint8_t online : 1;                  // bit 1 M
        uint8_t liftIsReversed : 1;          // bit 2 LF
        uint8_t liftIsPA : 1;                // bit 3 LF & PA
        uint8_t tiltIsPA : 1;                // bit 4 TL & PA
        uint8_t liftIsEncoderControlled : 1; // bit 5 LF & PA
        uint8_t tiltIsEncoderControlled : 1; // bit 6 LF & PA
    };
    static_assert(sizeof(ConfigStatus) == sizeof(uint8_t), "ConfigStatus Size is not correct");

    // Match directly with OperationalStatus 2 bits Fields
    enum class OperationalState : uint8_t
    {
        Stall             = 0x00, // currently not moving
        MovingUpOrOpen    = 0x01, // is currently opening
        MovingDownOrClose = 0x02, // is currently closing
        Reserved          = 0x03, // dont use
    };
    static_assert(sizeof(OperationalState) == sizeof(uint8_t), "OperationalState Size is not correct");

    struct OperationalStatus
    {
        OperationalState global : 2; // bit 0-1 M
        OperationalState lift : 2;   // bit 2-3 LF
        OperationalState tilt : 2;   // bit 4-5 TL
    };
    static_assert(sizeof(OperationalStatus) == sizeof(uint8_t), "OperationalStatus Size is not correct");

    struct SafetyStatus
    {
        uint8_t remoteLockout : 1;       // bit 0
        uint8_t tamperDetection : 1;     // bit 1
        uint8_t failedCommunication : 1; // bit 2
        uint8_t positionFailure : 1;     // bit 3
        uint8_t thermalProtection : 1;   // bit 4
        uint8_t obstacleDetected : 1;    // bit 5
        uint8_t powerIssue : 1;          // bit 6
        uint8_t stopInput : 1;           // bit 7
        uint8_t motorJammed : 1;         // bit 8
        uint8_t hardwareFailure : 1;     // bit 9
        uint8_t manualOperation : 1;     // bit 10
    };
    static_assert(sizeof(SafetyStatus) == sizeof(uint16_t), "SafetyStatus Size is not correct");

    template <class ActuatorImpl>
    class Actuator
    {
    public:
        Actuator(chip::EndpointId endpoint) : mEndpoint(endpoint) {}

        constexpr void OpenLimitSet(uint16_t limit) { Impl()->OpenLimitSet(limit); }
        constexpr uint16_t OpenLimitGet() { return Impl()->OpenLimitGet(); }

        constexpr void ClosedLimitSet(uint16_t limit) { Impl()->ClosedLimitSet(limit); }
        constexpr uint16_t ClosedLimitGet() { return Impl()->ClosedLimitGet(); }

        constexpr void PositionSet(uint16_t percent100ths) { Impl()->PositionSet(percent100ths); }
        constexpr uint16_t PositionGet() { return Impl()->PositionGet(); }

        void PositionValueSet(uint16_t value)
        {
            uint16_t percent100ths = ValueToPercent100ths(value);
            PositionSet(percent100ths);
        }

        uint16_t PositionValueGet()
        {
            uint16_t percent100ths = PositionGet();
            return Percent100thsToValue(percent100ths);
        }
        constexpr void TargetSet(uint16_t percent100ths) { Impl()->TargetSet(percent100ths); }
        constexpr uint16_t TargetGet() { return Impl()->TargetGet(); }

        void TargetValueSet(uint16_t value)
        {
            uint16_t percent100ths = ValueToPercent100ths(value);
            TargetSet(percent100ths);
        }

        uint16_t TargetValueGet()
        {
            uint16_t percent100ths = TargetGet();
            return Percent100thsToValue(percent100ths);
        }

        constexpr void NumberOfActuationsIncrement() { Impl()->NumberOfActuationsIncrement(); }
        constexpr uint16_t NumberOfActuationsGet() { return Impl()->NumberOfActuationsGet(); }

    protected:
        uint16_t ValueToPercent100ths(uint16_t position);
        uint16_t Percent100thsToValue(uint16_t percent100ths);

        chip::EndpointId mEndpoint;

    private:
        constexpr ActuatorImpl * Impl() { return static_cast<ActuatorImpl *>(this); }
    };

    class LiftActuator : public Actuator<LiftActuator>
    {
    public:
        LiftActuator(chip::EndpointId endpoint) : Actuator<LiftActuator>(endpoint) {}
        void OpenLimitSet(uint16_t limit);
        uint16_t OpenLimitGet();
        void ClosedLimitSet(uint16_t limit);
        uint16_t ClosedLimitGet();
        void PositionSet(uint16_t percent100ths);
        uint16_t PositionGet();
        void TargetSet(uint16_t percent100ths);
        uint16_t TargetGet();
        void NumberOfActuationsIncrement();
        uint16_t NumberOfActuationsGet();
    };

    class TiltActuator : public Actuator<TiltActuator>
    {
    public:
        TiltActuator(chip::EndpointId endpoint) : Actuator<TiltActuator>(endpoint) {}
        void OpenLimitSet(uint16_t limit);
        uint16_t OpenLimitGet();
        void ClosedLimitSet(uint16_t limit);
        uint16_t ClosedLimitGet();
        void PositionSet(uint16_t percent100ths);
        uint16_t PositionGet();
        void TargetSet(uint16_t percent100ths);
        uint16_t TargetGet();
        void NumberOfActuationsIncrement();
        uint16_t NumberOfActuationsGet();
    };

    static WindowCover & Instance();

    bool hasFeature(Feature feat) { return (mFeatures & (uint8_t) feat) > 0; }

    LiftActuator & Lift() { return mLift; }
    TiltActuator & Tilt() { return mTilt; }

    // Attribute: Id  0 Type
    void TypeSet(EmberAfWcType type);
    EmberAfWcType TypeGet();

    // Attribute: Id  7 ConfigStatus
    void ConfigStatusSet(const ConfigStatus & status);
    const ConfigStatus ConfigStatusGet();

    // Attribute: Id 10 OperationalStatus
    void OperationalStatusSet(const OperationalStatus & status);
    const OperationalStatus OperationalStatusGet();

    // Attribute: Id 13 EndProductType
    void EndProductTypeSet(EmberAfWcEndProductType endProduct);
    EmberAfWcEndProductType EndProductTypeGet();

    // Attribute: Id 24 Mode
    void ModeSet(const Mode & mode);
    const Mode ModeGet();

    // OPTIONAL Attributes -- Setter/Getter Internal Variables equivalent
    // Attribute: Id 27 SafetyStatus (Optional)
    void SafetyStatusSet(SafetyStatus & status);
    const SafetyStatus SafetyStatusGet();

    bool IsOpen();
    bool IsClosed();

protected:
    OperationalState ValueToOperationalState(uint8_t value);
    uint8_t OperationalStateToValue(const OperationalState & state);

    static constexpr chip::EndpointId mEndpoint = 1;

private:
    WindowCover() : mLift(mEndpoint), mTilt(mEndpoint) {}

    static WindowCover sInstance;
    uint8_t mFeatures = ((uint8_t) Feature::Lift | (uint8_t) Feature::Tilt | (uint8_t) Feature::PositionAware);
    LiftActuator mLift;
    TiltActuator mTilt;
};
