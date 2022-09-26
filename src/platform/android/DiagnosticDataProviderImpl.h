/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 *    @file
 *          Provides an implementation of the DiagnosticDataProvider object.
 */

#pragma once

#include <jni.h>
#include <platform/DiagnosticDataProvider.h>

namespace {
constexpr int offPremiseServicesReachableUnknown = 0;
constexpr int offPremiseServicesReachableYes     = 1;
constexpr int offPremiseServicesReachableNo      = 2;
} // namespace

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the PlatformManager singleton object for Linux platforms.
 */
class DiagnosticDataProviderImpl : public DiagnosticDataProvider
{
public:
    void InitializeWithObject(jobject managerObject);

public:
    static DiagnosticDataProviderImpl & GetDefaultInstance();
    CHIP_ERROR GetRebootCount(uint16_t & rebootCount) override;
    CHIP_ERROR GetNetworkInterfaces(NetworkInterface ** netifpp) override;
    void ReleaseNetworkInterfaces(NetworkInterface * netifp) override;

    jobject mDiagnosticDataProviderManagerObject = nullptr;
    jmethodID mGetRebootCountMethod              = nullptr;
    jmethodID mGetNifMethod                      = nullptr;
};

/**
 * Returns the platform-specific implementation of the DiagnosticDataProvider singleton object.
 *
 * Applications can use this to gain access to features of the DiagnosticDataProvider
 * that are specific to the selected platform.
 */
DiagnosticDataProvider & GetDiagnosticDataProviderImpl();

} // namespace DeviceLayer
} // namespace chip
