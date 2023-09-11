/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/clusters/mode-select-server/supported-modes-manager.h>
#include <app/util/af.h>
#include <app/util/config.h>
#include <cstring>

namespace chip {
namespace app {
namespace Clusters {
namespace ModeSelect {

/**
 * This implementation statically defines the options.
 */

class StaticSupportedModesManager : public chip::app::Clusters::ModeSelect::SupportedModesManager
{
    using ModeOptionStructType = Structs::ModeOptionStruct::Type;
    using storage_value_type   = const ModeOptionStructType;

    struct EndpointSpanPair
    {
        const EndpointId mEndpointId;
        const Span<storage_value_type> mSpan;

        EndpointSpanPair(const EndpointId aEndpointId, const Span<storage_value_type> && aSpan) :
            mEndpointId(aEndpointId), mSpan(aSpan)
        {}

        EndpointSpanPair() : mEndpointId(0), mSpan(Span<storage_value_type>()) {}
    };

    static storage_value_type coffeeOptions[];
    static const EndpointSpanPair supportedOptionsByEndpoints[EMBER_AF_MODE_SELECT_CLUSTER_SERVER_ENDPOINT_COUNT];

public:
    static const StaticSupportedModesManager instance;

    SupportedModesManager::ModeOptionsProvider getModeOptionsProvider(EndpointId endpointId) const override;

    Protocols::InteractionModel::Status getModeOptionByMode(EndpointId endpointId, uint8_t mode,
                                                            const ModeOptionStructType ** dataPtr) const override;

    ~StaticSupportedModesManager(){};

    StaticSupportedModesManager() {}

    static inline const StaticSupportedModesManager & getStaticSupportedModesManagerInstance() { return instance; }
};

const SupportedModesManager * getSupportedModesManager();

} // namespace ModeSelect
} // namespace Clusters
} // namespace app
} // namespace chip
