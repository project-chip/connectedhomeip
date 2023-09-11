/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
