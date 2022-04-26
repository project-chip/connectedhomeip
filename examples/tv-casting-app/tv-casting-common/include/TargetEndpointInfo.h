/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "LinuxCommissionableDataProvider.h"
#include "Options.h"
#include "app/clusters/bindings/BindingManager.h"
#include <app/OperationalDeviceProxy.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <controller/CHIPCommissionableNodeController.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/SafeInt.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/ConfigurationManager.h>
#include <platform/DeviceControlServer.h>
#include <platform/TestOnlyCommissionableDataProvider.h>
#include <system/SystemClock.h>
#include <system/SystemLayer.h>
#include <transport/raw/PeerAddress.h>
#include <zap-generated/CHIPClusters.h>

#include <list>
#include <string>

using namespace chip;
using namespace chip::Controller;
using namespace chip::Credentials;
using chip::ArgParser::HelpOptions;
using chip::ArgParser::OptionDef;
using chip::ArgParser::OptionSet;
using namespace chip::app::Clusters::ContentLauncher::Commands;

class TargetEndpointInfo
{
public:
    void Initialize(EndpointId endpointId);
    void Reset() { mInitialized = false; }
    bool IsInitialized() { return mInitialized; }
    EndpointId GetEndpointId() const { return mEndpointId; }

    bool HasCluster(ClusterId clusterId);
    bool AddCluster(ClusterId clusterId);
    void PrintInfo();

private:
    static constexpr size_t kMaxNumberOfClustersPerEndpoint = 10;
    ClusterId mClusters[kMaxNumberOfClustersPerEndpoint]    = {};
    EndpointId mEndpointId;
    bool mInitialized = false;
};