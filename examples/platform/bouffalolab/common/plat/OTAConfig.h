/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/DefaultOTARequestor.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorDriver.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorStorage.h>

class OTAConfig
{
public:
    OTAConfig(){};

    static void Init();
    static constexpr uint32_t kInitOTARequestorDelaySec = 3;
    static void InitOTARequestorHandler(chip::System::Layer * systemLayer, void * appState);
};
