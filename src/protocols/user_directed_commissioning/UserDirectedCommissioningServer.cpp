/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *      This file implements an object for a CHIP Echo unsolicitied
 *      responder (server).
 *
 */

#include "UserDirectedCommissioning.h"

namespace chip {
namespace Protocols {
namespace UserDirectedCommissioning {

CHIP_ERROR UserDirectedCommissioningServer::Init(Messaging::ExchangeManager * exchangeMgr)
{
    printf("UserDirectedCommissioningServer::Init");
    // Error if already initialized.
    if (mExchangeMgr != nullptr)
        return CHIP_ERROR_INCORRECT_STATE;

    mExchangeMgr = exchangeMgr;
    mHelper      = nullptr;

    // Register to receive unsolicited Echo Request messages from the exchange manager.
    mExchangeMgr->RegisterUnsolicitedMessageHandlerForType(MsgType::IdentificationDeclaration, this);

    printf("UserDirectedCommissioningServer::Init done");
    return CHIP_NO_ERROR;
}

void UserDirectedCommissioningServer::Shutdown()
{
    if (mExchangeMgr != nullptr)
    {
        mExchangeMgr->UnregisterUnsolicitedMessageHandlerForType(MsgType::IdentificationDeclaration);
        mExchangeMgr = nullptr;
    }
}

void UserDirectedCommissioningServer::OnMessageReceived(Messaging::ExchangeContext * ec, const PacketHeader & packetHeader,
                                                        const PayloadHeader & payloadHeader, System::PacketBufferHandle && payload)
{
    // payload->DebugDump("UserDirectedCommissioningServer::OnMessageReceive");

    char instanceName[USER_DIRECTED_COMMISSIONING_MAX_INSTANCE_NAME];
    int instanceNameLength = (payload->DataLength() > (USER_DIRECTED_COMMISSIONING_MAX_INSTANCE_NAME - 1))
        ? USER_DIRECTED_COMMISSIONING_MAX_INSTANCE_NAME - 1
        : payload->DataLength();
    payload->Read((uint8_t *) instanceName, instanceNameLength);

    instanceName[instanceNameLength] = '\0';

    // printf("UserDirectedCommissioningServer::OnMessageReceived instance=%s\n", instanceName);

    UDCClientState * client = mUdcClients.FindUDCClientState(instanceName, nullptr);
    if (client == nullptr)
    {
        printf("UserDirectedCommissioningServer::OnMessageReceived new instance state received\n");

        CHIP_ERROR err;
        err = mUdcClients.CreateNewUDCClientState(instanceName, &client);
        if (err != CHIP_NO_ERROR)
        {
            printf("UserDirectedCommissioningServer::OnMessageReceived error creating new connection state\n");
            return;
        }
        if (client == nullptr)
        {
            printf("UserDirectedCommissioningServer::OnMessageReceived no memory\n");
            return;
        }

        // Call the registered UDCHelper, if any.
        if (mHelper != nullptr)
        {
            mHelper->FindCommissionableNode(ec, instanceName);
        }
    }
    // else
    // {
    //     printf("UDC Received existing instance: name=%s, expiration=%lu state=%d\n", client->GetInstanceName(),
    //            client->GetExpirationTimeMs(), (int) client->GetUDCClientProcessingState());
    // }

    mUdcClients.MarkUDCClientActive(client);

    // Discard the exchange context.
    ec->Close();
}

void UserDirectedCommissioningServer::SetUDCClientProcessingState(char * instanceName, UDCClientProcessingState state)
{
    UDCClientState * client = mUdcClients.FindUDCClientState(instanceName, nullptr);
    if (client == nullptr)
    {
        // printf("SetUDCClientProcessingState new instance state received\n");
        CHIP_ERROR err;
        err = mUdcClients.CreateNewUDCClientState(instanceName, &client);
        if (err != CHIP_NO_ERROR)
        {
            printf("UserDirectedCommissioningServer::SetUDCClientProcessingState error creating new connection state\n");
            return;
        }
        if (client == nullptr)
        {
            printf("UserDirectedCommissioningServer::SetUDCClientProcessingState no memory\n");
            return;
        }
    }
    // else
    // {
    //     printf("SetUDCClientProcessingState existing instance: name=%s, expiration=%lu state=%d\n", client->GetInstanceName(),
    //            client->GetExpirationTimeMs(), (int) client->GetUDCClientProcessingState());
    // }

    printf("SetUDCClientProcessingState instance=%s new state=%d\n", instanceName, (int) state);

    client->SetUDCClientProcessingState(state);

    mUdcClients.MarkUDCClientActive(client);

    return;
}

void UserDirectedCommissioningServer::OnCommissionableNodeFound(const Mdns::CommissionableNodeData & nodeData)
{
    UDCClientState * client = mUdcClients.FindUDCClientState(nodeData.instanceName, nullptr);
    if (client != nullptr && client->GetUDCClientProcessingState() == UDCClientProcessingState::kDiscoveringNode)
    {
        printf("OnCommissionableNodeFound instance: name=%s, expiration=%lu old_state=%d new_state=%d\n", client->GetInstanceName(),
               client->GetExpirationTimeMs(), (int) client->GetUDCClientProcessingState(),
               (int) UDCClientProcessingState::kPromptingUser);
        client->SetUDCClientProcessingState(UDCClientProcessingState::kPromptingUser);

        // Call the registered UDCHelper, if any.
        if (mHelper != nullptr)
        {
            mHelper->OnUserDirectedCommissioningRequest(nodeData);
        }
    }
}

} // namespace UserDirectedCommissioning
} // namespace Protocols
} // namespace chip
