/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/clusters/mode-select-server/supported-modes-manager.h>
#include <app/util/af.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ModeSelect {

class StaticSupportedModesManager : public chip::app::Clusters::ModeSelect::SupportedModesManager
{
private:
    using ModeOptionStructType = Structs::ModeOptionStruct::Type;
    using SemanticTag          = Structs::SemanticTagStruct::Type;
    static int mSize;

    static ModeOptionsProvider * epModeOptionsProviderList;

    void FreeSupportedModes(EndpointId endpointId) const;

    static const StaticSupportedModesManager instance;

public:
    // InitEndpointArray should be called only once in the application. Memory allocated to the
    // epModeOptionsProviderList will be needed for the lifetime of the program, so it's never deallocated.
    static CHIP_ERROR InitEndpointArray(int size);

    // DeInitEndpointArray should be called only when application need to reallocate memory of
    // epModeOptionsProviderList ( Eg. Bridges ).
    static void DeInitEndpointArray()
    {
        delete[] epModeOptionsProviderList;
        epModeOptionsProviderList = nullptr;
        mSize                     = 0;
    }

    SupportedModesManager::ModeOptionsProvider getModeOptionsProvider(EndpointId endpointId) const override;

    Protocols::InteractionModel::Status getModeOptionByMode(EndpointId endpointId, uint8_t mode,
                                                            const ModeOptionStructType ** dataPtr) const override;

    void CleanUp(EndpointId endpointId) const;

    StaticSupportedModesManager() {}

    ~StaticSupportedModesManager()
    {
        for (int i = 0; i < mSize; i++)
        {
            FreeSupportedModes(i);
        }
    }

    static inline const StaticSupportedModesManager & getStaticSupportedModesManagerInstance() { return instance; }
};

const SupportedModesManager * getSupportedModesManager();

} // namespace ModeSelect
} // namespace Clusters
} // namespace app
} // namespace chip
