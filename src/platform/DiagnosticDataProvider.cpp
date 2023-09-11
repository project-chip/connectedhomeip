/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Implements a getter and setter for a singleton DiagnosticDataProvider object.
 */

#include <lib/support/CodeUtils.h>
#include <platform/DiagnosticDataProvider.h>

namespace chip {
namespace DeviceLayer {

class DiagnosticDataProvider;

namespace {

/** Singleton pointer to the DiagnosticDataProvider implementation.
 */
DiagnosticDataProvider * gInstance = nullptr;

} // namespace

DiagnosticDataProvider & GetDiagnosticDataProvider()
{
    if (gInstance != nullptr)
    {
        return *gInstance;
    }

    return GetDiagnosticDataProviderImpl();
}

void SetDiagnosticDataProvider(DiagnosticDataProvider * diagnosticDataProvider)
{
    if (diagnosticDataProvider != nullptr)
    {
        gInstance = diagnosticDataProvider;
    }
}

} // namespace DeviceLayer
} // namespace chip
