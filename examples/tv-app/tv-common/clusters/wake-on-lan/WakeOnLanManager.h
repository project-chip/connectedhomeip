/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/clusters/wake-on-lan-server/wake-on-lan-server.h>

class WakeOnLanManager : public chip::app::Clusters::WakeOnLan::Delegate
{
public:
    CHIP_ERROR HandleGetMacAddress(chip::app::AttributeValueEncoder & aEncoder) override;
};
