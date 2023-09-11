/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/af-enums.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ModeSelect {

/**
 * Interface to help manage the supported modes of the Mode Select Cluster.
 */
class SupportedModesManager
{

    using ModeOptionStructType = Structs::ModeOptionStruct::Type;

public:
    /**
     * A class that can return the supported ModeOptions for a specific endpoint.
     */
    struct ModeOptionsProvider
    {
        using pointer = const ModeOptionStructType *;

        /**
         * Returns the ModeOptionStructIterator to the first option.
         */
        inline pointer begin() const { return mBegin; }

        /**
         * Returns the ModeOptionStructIterator to an element after the last option.
         */
        inline pointer end() const { return mEnd; }

        ModeOptionsProvider() : mBegin(nullptr), mEnd(nullptr) {}

        ModeOptionsProvider(const pointer aBegin, const pointer aEnd) : mBegin(aBegin), mEnd(aEnd) {}

        pointer mBegin;
        pointer mEnd;
    };

    /**
     * Given the endpointId, returns all its supported modes options.
     * @param endpointId
     * @return The mode options provider for the endpoint.
     */
    virtual ModeOptionsProvider getModeOptionsProvider(EndpointId endpointId) const = 0;

    /**
     * Given the endpointId and a mode value, find the ModeOptionStruct that matches the mode.
     * @param endpointId The endpoint for which to search the ModeOptionStruct.
     * @param mode The mode for which to search the ModeOptionStruct.
     * @param dataPtr The pointer to receive the ModeOptionStruct, if one is found.
     * @return Status::Success if successfully found the option. Otherwise, returns appropriate status code (found in
     * <app/util/af-enums.h>)
     */
    virtual Protocols::InteractionModel::Status getModeOptionByMode(EndpointId endpointId, uint8_t mode,
                                                                    const ModeOptionStructType ** dataPtr) const = 0;

    virtual ~SupportedModesManager() {}
};

const SupportedModesManager * getSupportedModesManager();

} // namespace ModeSelect
} // namespace Clusters
} // namespace app
} // namespace chip
