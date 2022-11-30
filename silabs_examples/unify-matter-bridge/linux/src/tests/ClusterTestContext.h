/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

/**
 * @file
 *   Define a helper template for testing client interactions.
 */

#ifndef CLUSTER_TEST_CONTEXT_H_
#define CLUSTER_TEST_CONTEXT_H_

#include "UnifyBridgeContext.h"
#include <optional>

// Chip components
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <controller/InvokeInteraction.h>
#include <controller/ReadInteraction.h>

// Third party library
#include <nlunit-test.h>

namespace unify::matter_bridge {
namespace Test {
/**
 * @brief
 *   By specializing <AttributeHandler> and <CommandHandler> parameters, this class
 *   registers attribute/command handlers accordingly and lets you assert the result
 *   of client interaction.
 *
 */
template <class AttributeHandler, class CommandHandler>
class ClusterContext : public UnifyBridgeContext
{
public:
    static const uint16_t kEndpointId = 2;
    static const std::string kNodeId;

    /**
     * @brief
     *   It's necessary to call this function as a part of your initial setup of the
     *   entire test suite.
     *
     * @param[inout]  inContext   A pointer to test suite-specific context
     *                            provided by the test suite driver.
     */
    static int Initialize(void * context)
    {
        if (UnifyBridgeContext::Initialize(context) != SUCCESS)
            return FAILURE;

        auto * ctx = static_cast<ClusterContext *>(context);

        if (!ctx->mAttributeHandler)
            ctx->mAttributeHandler.emplace(ctx->mNodeStateMonitor, ctx->mMqttHandler);

        if (!ctx->mCommandHandler)
            ctx->mCommandHandler.emplace(ctx->mNodeStateMonitor, ctx->mMqttHandler, ctx->mGroupTranslator);

        return SUCCESS;
    }

    /**
     * @brief
     *   Creates or gets a Unify endpoint. This class has a built-in Unify node
     *   ("zw-0x0002"). This method populates the node with a new endpoint and returns
     *   its reference.
     *
     */
    unify::node_state_monitor::endpoint & get_endpoint(uint16_t endpoint_id = ClusterContext::kEndpointId)
    {
        return mNode.emplace_endpoint(endpoint_id);
    }

    /**
     * @brief
     *   Call this method to conclude your initial setup. Returns SUCCESS when the passed
     *   endpoint has a proper mapping between Unify SDK and Matter.
     *
     */
    int register_endpoint(const unify::node_state_monitor::endpoint & endpoint)
    {
        mNodeStateMonitor.call_on_unify_node_added(mNode);
        auto bridged_ep = mNodeStateMonitor.bridged_endpoint(ClusterContext::kNodeId, endpoint.endpoint_id);

        if (bridged_ep == nullptr)
            return FAILURE;

        if (bridged_ep->matter_endpoint != endpoint.endpoint_id)
            return FAILURE;

        return SUCCESS;
    }

    /**
     * @brief
     *   The type parameter T is generally expected to be a
     *   ClusterName::Attributes::AttributeName::TypeInfo struct.
     *
     *   First the textual representation of the attribute value is sent to its MQTT
     *   topic, simulating the attribute update taking place in Unify SDK. Secondly a
     *   mocked CHIP client reads the attribute value and interprets in the desired
     *   format. It passes when the test scenario is happy and the resulting value
     *   matches.
     *
     *   For unsupported attributes or any erroneous conditions, you can inspect the
     *   retrun value of type CHIP_ERROR.
     */
    template <typename T, bool happy = true>
    inline CHIP_ERROR attribute_test(nlTestSuite * sSuite, const std::string & topic, const std::string & json_payload,
                                     typename T::Type value)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        mMqttHandler.subscribeCB(topic.c_str(), json_payload.c_str(), json_payload.length(), &mAttributeHandler.value());

        auto onSuccessCb = [sSuite, value](const chip::app::ConcreteDataAttributePath & attributePath,
                                           const typename T::Type & dataResponse) {
            if (happy)
            {
                NL_TEST_ASSERT(sSuite, dataResponse == value);
            }
            else
            {
                NL_TEST_ASSERT(sSuite, dataResponse != value);
            }
        };

        auto onFailureCb = [&err](const chip::app::ConcreteDataAttributePath * attributePath, CHIP_ERROR aError) { err = aError; };

        err = chip::Controller::ReadAttribute<T>(&GetExchangeManager(), GetSessionBobToAlice(), kEndpointId, onSuccessCb,
                                                 onFailureCb);
        NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
        DrainAndServiceIO();

        return err;
    }

    /**
     * @brief
     *   The type parameter T is generally expected to be a
     *   ClusterName::Commands::CommandName::Type struct.
     *
     *   First a mocked CHIP client issues the command. In response Unify Bridge
     *   publishes an event to the corresponding MQTT topic. It passes when the
     *   textual representation of the command matches.
     *
     *   For unsupported commands or any erroneous conditions, you can inspect the retrun
     *   value of type CHIP_ERROR.
     */
    template <typename T>
    inline CHIP_ERROR
    command_test(nlTestSuite * sSuite, const std::string & topic, const std::string & json_payload, T & request,
                 typename chip::Controller::TypedCommandCallback<typename T::ResponseType>::OnSuccessCallbackType onSuccessCb)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;

        auto onFailureCb = [&err](CHIP_ERROR aError) { err = aError; };

        err = chip::Controller::InvokeCommandRequest<T>(&GetExchangeManager(), GetSessionBobToAlice(), kEndpointId, request,
                                                        onSuccessCb, onFailureCb);
        NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
        DrainAndServiceIO();

        if (err == CHIP_NO_ERROR)
        {
            NL_TEST_ASSERT(sSuite, mMqttHandler.publish_topic == topic);
            NL_TEST_ASSERT(sSuite, mMqttHandler.publish_payload == json_payload);
        }

        return err;
    }

    template <typename T>
    inline CHIP_ERROR command_test(nlTestSuite * sSuite, const std::string & topic, const std::string & json_payload, T & request)
    {
        auto onSuccessCb = [sSuite](const chip::app::ConcreteCommandPath & commandPath, const chip::app::StatusIB & aStatus,
                                    const chip::app::DataModel::NullObjectType & dataResponse) { NL_TEST_ASSERT(sSuite, true); };
        return command_test<T>(sSuite, topic, json_payload, request, onSuccessCb);
    }

    /**
     * @brief
     *   @param[in] request is sent to Unify Bridge as a command.
     *   @param[out] response gets updated once the client receives a response of the command.
     */
    template <typename T>
    inline CHIP_ERROR command_test(nlTestSuite * sSuite, const std::string & topic, const std::string & json_payload, T & request,
                                   typename T::ResponseType & response)
    {
        auto onSuccessCb = [sSuite, &response](const chip::app::ConcreteCommandPath & commandPath,
                                               const chip::app::StatusIB & aStatus,
                                               const typename T::ResponseType & dataResponse) { response = dataResponse; };

        return command_test<T>(sSuite, topic, json_payload, request, onSuccessCb);
    }

private:
    std::optional<AttributeHandler> mAttributeHandler;
    std::optional<CommandHandler> mCommandHandler;
    unify::node_state_monitor::node mNode = unify::node_state_monitor::node(ClusterContext::kNodeId);
};

template <class AttributeHandler, class CommandHandler>
const std::string ClusterContext<AttributeHandler, CommandHandler>::kNodeId = "zw-0x0002";

} // namespace Test
} // namespace unify::matter_bridge

#endif // CLUSTER_TEST_CONTEXT_H_