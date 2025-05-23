/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/StatusResponse.h>
#include <app/reporting/reporting.h>

#include <app/SafeAttributePersistenceProvider.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/TypeTraits.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ZoneManagement {

class ZoneManagementServer;

/** @brief
 *  Defines interfaces for implementing application-specific logic for various aspects of the ZoneManagement Cluster.
 *  Specifically, it defines interfaces for the command handling and loading of the allocated streams.
 */
class ZoneManagementDelegate
{
public:
    ZoneManagementDelegate() = default;

    virtual ~ZoneManagementDelegate() = default;

private:
    friend class ZoneManagementServer;

    ZoneManagementServer * mZoneManagementServer = nullptr;

    /**
     * This method is used by the SDK to ensure the delegate points to the server instance it's associated with.
     * When a server instance is created or destroyed, this method will be called to set and clear, respectively,
     * the pointer to the server instance.
     *
     * @param aZoneManagementServer  A pointer to the ZoneManagementServer object related to this delegate object.
     */
    void SetZoneManagementServer(ZoneManagementServer * aZoneManagementServer)
    {
        mZoneManagementServer = aZoneManagementServer;
    }

protected:
    ZoneManagementServer * GetZoneManagementServer() const { return mZoneManagementServer; }
};

class ZoneManagementServer : public CommandHandlerInterface, public AttributeAccessInterface
{
public:

   ZoneManagementServer(ZoneManagementDelegate & aDelegate, EndpointId aEndpointId, const BitFlags<Feature> aFeatures,
                        const BitFlags<OptionalAttribute> aOptionalAttrs, uint8_t aMaxUserDefinedZones,
                        uint8_t aMaxZones, uint8_t aSensitivityMax, const TwoDCartesianVertexStruct & aTwoDCartesianMax);

   ~ZoneManagementServer() overrise;

    /**
     * @brief Initialise the Zone Management server instance.
     * This function must be called after defining a ZoneManagementServer class object.
     * @return Returns an error if the given endpoint and cluster ID have not been enabled in zap or if the
     * CommandHandler or AttributeHandler registration fails, else returns CHIP_NO_ERROR.
     * This method also checks if the feature setting is valid, if invalid it will return CHIP_ERROR_INVALID_ARGUMENT.
     */
    CHIP_ERROR Init();

    bool HasFeature(Feature feature) const;

    bool SupportsOptAttr(OptionalAttribute aOptionalAttr) const;

    // Attribute Setters

    // Attribute Getters

private:
    ZoneManagementDelegate & mDelegate;
    EndpointId mEndpointId;
    const BitFlags<Feature> mFeatures;
    const BitFlags<OptionalAttribute> mOptionalAttrs;

    // Attributes
    const uint8_t mMaxUserDefinedZones;
    const uint8_t mMaxZones;
    const uint8_t mSensitivityMax;
    const TwoDCartesianVertexStruct mTwoDCartesianMax;

    std::vector<ZoneInformationStruct> mZones;
    std::vector<ZoneTriggerControlStruct> mTriggers;
    uint8_t mSensitivity;

    /**
     * IM-level implementation of read
     * @return appropriately mapped CHIP_ERROR if applicable
     */
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    /**
     * IM-level implementation of write
     * @return appropriately mapped CHIP_ERROR if applicable
     */
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

    /**
     * @brief Inherited from CommandHandlerInterface
     */
    void InvokeCommand(HandlerContext & ctx) override;

};

} // namespace ZoneManagement
} // namespace Clusters
} // namespace app
} // namespace chip