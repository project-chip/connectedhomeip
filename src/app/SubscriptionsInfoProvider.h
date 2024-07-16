/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

/**
 *    @file
 *      This file defines an interface that exposes all the public subscription information APIs.
 *      The interface is implemented by the InteractionModelEngine to avoid creating unnecessary dependencies
 *      since the IMEngine has more dependency than its consummers need.
 *      By leveraging the SubscriptionInfoProvider APIs, a consumer avoids depending on the global data model functions.
 */

#pragma once

#include <lib/core/DataModelTypes.h>
#include <lib/core/NodeId.h>

namespace chip {
namespace app {

class SubscriptionsInfoProvider
{
public:
    virtual ~SubscriptionsInfoProvider(){};

    /**
     * @brief Check if a given subject (CAT or operational NodeId) has at least 1 active subscription.
     *
     * @param[in] aFabricIndex fabric index of the subject
     * @param[in] subjectID NodeId of the subject
     *            subjectID may encode a CAT in the reserved section of the NodeID.
     *
     * @return true subject has at least one active subscription with the device
     *         false subject doesn't have any active subscription with the device
     */
    virtual bool SubjectHasActiveSubscription(FabricIndex aFabricIndex, NodeId subjectID) = 0;

    /**
     * @brief Check if a given subject (CAT or operational NodeId) has at least 1 persisted subscription.
     *        See the CHIP_CONFIG_PERSIST_SUBSCRIPTIONS for more information on persisted subscriptions.
     *
     * @param[in] aFabricIndex fabric index of the subject
     * @param[in] subjectID NodeId of the subject
     *            subjectID may encode a CAT in the reserved section of the NodeID
     *
     * @return true subject has at least one persisted subscription with the device
     *         false subject doesn't have any persisted subscription with the device
     */
    virtual bool SubjectHasPersistedSubscription(FabricIndex aFabricIndex, NodeId subjectID) = 0;
};

} // namespace app
} // namespace chip
