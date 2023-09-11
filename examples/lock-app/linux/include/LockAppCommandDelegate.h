/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <NamedPipeCommands.h>
#include <json/json.h>

#include <app/clusters/door-lock-server/door-lock-server.h>

class LockAppCommandDelegate : public NamedPipeCommandDelegate
{
public:
    void OnEventCommandReceived(const char * json) override;
};
