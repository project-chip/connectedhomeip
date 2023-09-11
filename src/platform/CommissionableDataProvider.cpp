/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lib/support/CodeUtils.h>
#include <platform/CommissionableDataProvider.h>

namespace chip {
namespace DeviceLayer {

namespace {

CommissionableDataProvider * gCommissionableDataProvider = nullptr;

} // namespace

CommissionableDataProvider * GetCommissionableDataProvider()
{
    VerifyOrDie(gCommissionableDataProvider != nullptr);
    return gCommissionableDataProvider;
}

void SetCommissionableDataProvider(CommissionableDataProvider * provider)
{
    if (provider == nullptr)
    {
        return;
    }

    gCommissionableDataProvider = provider;
}

} // namespace DeviceLayer
} // namespace chip
