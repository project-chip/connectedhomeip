/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af-types.h>
#include <app/util/basic-types.h>
#include <platform/CHIPDeviceConfig.h>

class RefrigeratorAlarmServer
{
public:
    static RefrigeratorAlarmServer & Instance();

    EmberAfStatus GetMaskValue(chip::EndpointId endpoint, chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> * mask);
    EmberAfStatus GetStateValue(chip::EndpointId endpoint, chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> * state);
    EmberAfStatus GetSupportedValue(chip::EndpointId endpoint,
                                    chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> * suppported);

    // A change in mask value will result in a corresponding change in state.
    EmberAfStatus SetMaskValue(chip::EndpointId endpoint,
                               const chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> mask);

    // When State changes we are generating Notify event.
    EmberAfStatus SetStateValue(chip::EndpointId endpoint,
                                chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> newState);

    // A change in supported value will result in a corresponding change in mask and state.
    EmberAfStatus SetSupportedValue(chip::EndpointId endpoint,
                                    const chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> supported);

private:
    static RefrigeratorAlarmServer instance;

    void SendNotifyEvent(chip::EndpointId endpointId, chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> becameActive,
                         chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> becameInactive,
                         chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> newState,
                         chip::BitMask<chip::app::Clusters::RefrigeratorAlarm::AlarmMap> mask);
};
