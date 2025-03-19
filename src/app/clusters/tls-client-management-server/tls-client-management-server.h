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
#include <app/ConcreteAttributePath.h>
#include <app/reporting/reporting.h>
#include <lib/core/CHIPError.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {

class TlsClientManagementDelegate;

class TlsClientManagementServer : private AttributeAccessInterface, private CommandHandlerInterface
{
public:
    /**
     * Creates a TLSClientManagement server instance. The Init() function needs to be called for this instance to be registered and
     * called by the interaction model at the appropriate times.
     * @param endpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param delegate A reference to the delegate to be used by this server.
     * @param certificateTable A reference to the certificate table for looking up certiciates
     * Note: the caller must ensure that the delegate lives throughout the instance's lifetime.
     */
    TlsClientManagementServer(EndpointId endpointId, TlsClientManagementDelegate & delegate, CertificateTable & certificateTable);
    ~TlsClientManagementServer();

    /**
     * Initialise the TLS Client Management server instance.
     * @return Returns an error  if the CommandHandler or AttributeHandler registration fails.
     */
    CHIP_ERROR Init();

    // Attribute Setters

    /**
     * Sets the MaxProvisioned attribute. Note, this also handles writing the new value into non-volatile storage.
     * @param maxProvisioned The value to which maxProvisioned is to be set.
     */
    Protocols::InteractionModel::Status SetMaxProvisioned(uint8_t maxProvisioned);

    // Attribute Getters

    /**
     * @return The MaxProvisioned attribute.
     */
    uint8_t GetMaxProvisioned() const;

    /**
     * @return The endpoint ID.
     */
    EndpointId GetEndpointId() { return AttributeAccessInterface::GetEndpointId().Value(); }

private:
    TlsClientManagementDelegate & mDelegate;
    CertificateTable & mCertificateTable;

    // Attribute local storage
    uint8_t mMaxProvisioned;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & ctx) override;

    // Helpers
    // Loads all the persistent attributes from the KVS.
    void LoadPersistentAttributes();

    // Encodes all provisioned endpoints
    CHIP_ERROR EncodeProvisionedEndpoints(const AttributeValueEncoder::ListEncodeHelper & encoder);
};

/** @brief
 *  Defines methods for implementing application-specific logic for the TLSClientManagement Cluster.
 */
class TlsClientManagementDelegate
{
public:
    TlsClientManagementDelegate() = default;

    virtual ~TlsClientManagementDelegate() = default;

    /**
     * @brief Get the TLSEndpointStruct at a given index
     *
     * @param[in] index The index of the endpoint in the list.
     * @param[out] endpoint The endpoint at the given index in the list.
     * @return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the index is out of range for the preset types list.
     */
    virtual CHIP_ERROR GetProvisionedEndpointByIndex(size_t index, Structs::TLSEndpointStruct::Type & endpoint) = 0;

    /**
     * @brief Finds the TLSEndpointStruct with the given EndpointID
     *
     * @param[in] endpointID The EndpoitnID to find.
     * @param[out] endpoint The endpoint at the given index in the list.
     * @return NOT_FOUND if no mapping is found.
     */
    virtual CHIP_ERROR FindProvisionedEndpointByID(uint16_t endpointID, Structs::TLSEndpointStruct::Type & endpoint) = 0;

    /**
     * @brief Appends a TLSEndpointStruct to the provisioned endpoints list maintained by the delegate.
     *        The delegate must ensure it makes a copy of the provided preset and the data
     *        of its preset handle, if any.
     *
     * @param[in] endpoint The endpoint to add to the list.
     * @param[out] endpointID a reference to the uint16_t variable that is to contain the EndpointID value.
     *
     * @return CHIP_NO_ERROR if the endpoint was appended to the list successfully.
     * @return CHIP_ERROR if there was an error adding the endpoint to the list.
     */
    virtual CHIP_ERROR AppendToProvisionedEndpointList(const Structs::TLSEndpointStruct::Type & endpoint, uint16_t & endpointID) = 0;

    /**
     * @brief Removes the TLSEndpointStruct with the given EndpointID
     *
     * @param[in] endpointID The ID of the endpoint to remove.
     * @return NOT_FOUND if no mapping is found.
     */
    virtual CHIP_ERROR RemoveProvisionedEndpointByID(uint16_t endpointID) = 0;

protected:
    friend class TlsClientManagementServer;

    TlsClientManagementServer * mTlsClientManagementServer = nullptr;

    // sets the TlsClientManagement Server pointer
    void SetTlsClientManagementServer(TlsClientManagementServer * tlsClientManagementServer)
    {
        mTlsClientManagementServer = tlsClientManagementServer;
    }
    TlsClientManagementServer * GetTlsClientManagementServer() const { return mTlsClientManagementServer; }
};

} // namespace Clusters
} // namespace app
} // namespace chip
