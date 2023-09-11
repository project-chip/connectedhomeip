/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
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
    AlarmCodeEnum alarmCode;
};

class DoorStateEventData : public EventData
{
public:
    DoorStateEnum doorState;
};

CHIP_ERROR RegisterLockEvents();
void EventWorkerFunction(intptr_t context);
