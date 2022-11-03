/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <app/clusters/door-lock-server/door-lock-server.h>

class EventData
{
public:
    chip::EventId eventId;
};

class AlarmEventData : public EventData
{
public:
    DlAlarmCode alarmCode;
};

class DoorStateEventData : public EventData
{
public:
    DlDoorState doorState;
};

CHIP_ERROR RegisterLockEvents();
void EventWorkerFunction(intptr_t context);
