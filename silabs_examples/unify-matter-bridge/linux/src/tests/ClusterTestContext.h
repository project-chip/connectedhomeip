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
#include <app/reporting/tests/MockReportScheduler.h>

// Chip components
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <controller/InvokeInteraction.h>
#include <controller/ReadInteraction.h>

// Third party library
#include <nlunit-test.h>

#include "TestHelpers.hpp"

template <typename T>
struct is_nullable : std::false_type {
};

template <typename T>
struct is_nullable<chip::app::DataModel::Nullable<T>> : std::true_type {
};

namespace unify::matter_bridge {
namespace Test {

static uint8_t gDebugEventBuffer[120];
static uint8_t gInfoEventBuffer[120];
static uint8_t gCritEventBuffer[120];
static chip::app::CircularEventBuffer gCircularEventBuffer[3];
auto * engine = chip::app::InteractionModelEngine::GetInstance();

/** @brief Poor man's Result<T, CHIP_ERROR>. */
template <class T>
class Result
{
public:
    Result() : mOk(), mError(CHIP_NO_ERROR) {}

    void set_value(T value) { mOk = value; }

    void set_error(CHIP_ERROR err) { mError = err; }

    T unwrap() { return mOk.value(); }

    CHIP_ERROR unwrap_err() { return mError; }

private:
    std::optional<T> mOk;
    CHIP_ERROR mError;
};

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
    chip::MonotonicallyIncreasingCounter<chip::EventNumber> mEventCounter;

    /**
     * @brief
     *   It's necessary to call this function as a part of your initial setup of the
     *   entire test suite.
     *
     * @param[in]  Context   A pointer to test suite-specific context
     *                            provided by the test suite driver.
     * @param[in]  enableEvents Boolean to enable event handling for the test suite
     */
    static int Initialize(void * context, bool enableEvents)
    {
        auto * ctx = static_cast<ClusterContext *>(context);
        
        if (ctx->UMB_Initialize() != CHIP_NO_ERROR)
            return FAILURE;
        
        if (ctx->nlTestSetUp(context) != SUCCESS)
            return FAILURE;

        if (!ctx->mAttributeHandler)
            ctx->mAttributeHandler.emplace(ctx->mNodeStateMonitor, ctx->mMqttHandler, ctx->mDeviceTranslator);

        if (!ctx->mCommandHandler)
            ctx->mCommandHandler.emplace(ctx->mNodeStateMonitor, ctx->mMqttHandler,
                                            ctx->mGroupTranslator, ctx->mDeviceTranslator);

        if (enableEvents)
        {
            if (ctx->mEventCounter.Init(0) != CHIP_NO_ERROR)
            {
                return FAILURE;
            }

            chip::app::LogStorageResources logStorageResources[] = {
                { &gDebugEventBuffer[0], sizeof(gDebugEventBuffer), chip::app::PriorityLevel::Debug },
                { &gInfoEventBuffer[0], sizeof(gInfoEventBuffer), chip::app::PriorityLevel::Info },
                { &gCritEventBuffer[0], sizeof(gCritEventBuffer), chip::app::PriorityLevel::Critical },
            };

            chip::app::EventManagement::CreateEventManagement(&ctx->GetExchangeManager(),
                ArraySize(logStorageResources), gCircularEventBuffer, logStorageResources, &ctx->mEventCounter);
    
            engine->Init(&ctx->GetExchangeManager(), &ctx->GetFabricTable(),
                            chip::app::reporting::GetDefaultReportScheduler());
        }
        return SUCCESS;
    }

    static int Initialize(void * context)
    {
        if (Initialize(context, false) != SUCCESS)
            return FAILURE;
 
        return SUCCESS;
    }

    /**
     * @brief
     *   It's necessary to call this function as a part of your cleanup of the
     *   entire test suite.
     *
     * @param[in]  Context   A pointer to test suite-specific context
     *                            provided by the test suite driver.
     * @param[in]  enableEvents Boolean to enable event handling for the test suite
     */
    static int Finalize(void * context, bool enableEvents)
    {
        auto * ctx = static_cast<ClusterContext *>(context);
        
        if(enableEvents)
        {
            chip::app::EventManagement::DestroyEventManagement();
            engine->Shutdown();
        }    
        
        if (ctx->nlTestTearDown(context) != SUCCESS)
            return FAILURE;
            
        ctx->UMB_Finalize();

        return SUCCESS;
    }

    static int Finalize(void * context)
    {
        if (Finalize(context, false) != SUCCESS)
        {
            return FAILURE;
        }

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

    void  mqtt_subscribeCb(const std::string & topic, const std::string & json_payload)
    {
        mMqttHandler.subscribeCB(topic.c_str(), json_payload.c_str(), json_payload.length(), &mCommandHandler.value());
    }
    
    template <typename DecodableEventType>
    inline CHIP_ERROR event_test(nlTestSuite * sSuite, chip::EventNumber eventNumber, DecodableEventType & eventData)
    {
        auto sessionHandle      = GetSessionBobToAlice();
        bool onSuccessCbInvoked = false, onFailureCbInvoked = false;
        CHIP_ERROR err    = CHIP_NO_ERROR;

        // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. 
        // Otherwise, it's not safe to do so.
        auto onSuccessCb = [&eventNumber, &eventData, &onSuccessCbInvoked](const chip::app::EventHeader & eventHeader,
                                                                            const auto & eventResponse)
        {
            if (eventHeader.mEventNumber == eventNumber)
            {
                eventData = eventResponse;
                onSuccessCbInvoked = true;
            }
        };

        // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. 
        // Otherwise, it's not safe to do so.
        auto onFailureCb = [&eventNumber, &err, &onFailureCbInvoked](const chip::app::EventHeader * eventHeader, CHIP_ERROR aError)
        {
            if (eventHeader->mEventNumber == eventNumber)
            {
                err = aError;
                onFailureCbInvoked = true;
            }
        };

        err = chip::Controller::SubscribeEvent<DecodableEventType>(
                    &GetExchangeManager(), sessionHandle, kEndpointId, onSuccessCb, onFailureCb, 0, 5);
        NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
        
        DrainAndServiceIO();
        NL_TEST_ASSERT(sSuite, !onFailureCbInvoked);
        NL_TEST_ASSERT(sSuite, onSuccessCbInvoked);
        NL_TEST_ASSERT(sSuite, GetExchangeManager().GetNumActiveExchanges() == 0);

        return err;
    }

    template <typename T>
    inline CHIP_ERROR attribute_test(
        nlTestSuite * sSuite, const std::string & topic, const std::string & json_payload,
        typename chip::Controller::TypedReadAttributeCallback<typename T::DecodableType>::OnSuccessCallbackType onSuccessCb)
    {
        CHIP_ERROR err   = CHIP_NO_ERROR;
        auto onFailureCb = [&err](const chip::app::ConcreteDataAttributePath * attributePath, CHIP_ERROR aError) { err = aError; };
        err = chip::Controller::ReadAttribute<T>(&GetExchangeManager(), GetSessionBobToAlice(), kEndpointId, onSuccessCb,
                                                 onFailureCb);
        DrainAndServiceIO();

        return err;
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
        // CHIP_ERROR err = CHIP_NO_ERROR;
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

        return attribute_test<T>(sSuite, topic, json_payload, onSuccessCb);
    }

    /**
     * @brief
     *   @return A result containing the attribute value or error code.
     */
    template <typename T>
    inline Result<typename T::Type> attribute_test(nlTestSuite * sSuite, const std::string & topic,
                                                   const std::string & json_payload)
    {
        Result<typename T::Type> result;
        auto onSuccessCb = [&result](const chip::app::ConcreteAttributePath & attributePath,
                                     const typename T::Type & dataResponse) { result.set_value(dataResponse); };
        CHIP_ERROR err   = attribute_test<T>(sSuite, topic, json_payload, onSuccessCb);
        result.set_error(err);

        return result;
    }

    template <typename T>
    inline void attribute_write_test(nlTestSuite * sSuite, const std::string & topic, const std::string & json_payload,
                                     typename T::Type value)
    {

        auto sessionHandle = GetSessionBobToAlice();

        bool onSuccessCbInvoked = false;
        bool onFailureCbInvoked = false;

        mMqttHandler.reset();

        auto onSuccessCb = [&onSuccessCbInvoked](const chip::app::ConcreteAttributePath & attributePath) {
            onSuccessCbInvoked = true;
        };
        auto onFailureCb = [&onFailureCbInvoked](const chip::app::ConcreteAttributePath * attributePath, CHIP_ERROR aError) {
            onFailureCbInvoked = true;
        };

        chip::Controller::WriteAttribute<T>(sessionHandle, kEndpointId, value, onSuccessCb, onFailureCb);

        DrainAndServiceIO();

        NL_TEST_ASSERT_EQUAL(sSuite, true, onSuccessCbInvoked);
        NL_TEST_ASSERT_EQUAL(sSuite, false, onFailureCbInvoked);
        NL_TEST_ASSERT_EQUAL_JSON(sSuite, json_payload, mMqttHandler.publish_payload);
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
                 typename chip::Controller::TypedCommandCallback<typename T::ResponseType>::OnSuccessCallbackType onSuccessCb,
                 const chip::Optional<uint16_t> & timedInvokeTimeoutMs)
    {
        CHIP_ERROR err   = CHIP_NO_ERROR;
        auto onFailureCb = [&err](CHIP_ERROR aError) { err = aError; };

        err = chip::Controller::InvokeCommandRequest<T>(&GetExchangeManager(), GetSessionBobToAlice(), kEndpointId, request,
                                                        onSuccessCb, onFailureCb, timedInvokeTimeoutMs);
        NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
        DrainAndServiceIO();

        if (err == CHIP_NO_ERROR)
        {
            NL_TEST_ASSERT(sSuite, mMqttHandler.publish_topic == topic);
            NL_TEST_ASSERT_EQUAL_JSON(sSuite, json_payload, mMqttHandler.publish_payload);
        }

        return err;
    }

    template <typename T>
    inline CHIP_ERROR command_test(nlTestSuite * sSuite, const std::string & topic, const std::string & json_payload, T & request)
    {
        auto onSuccessCb = [sSuite](const chip::app::ConcreteCommandPath & commandPath, const chip::app::StatusIB & aStatus,
                                    const chip::app::DataModel::NullObjectType & dataResponse) { NL_TEST_ASSERT(sSuite, true); };
        return command_test<T>(sSuite, topic, json_payload, request, onSuccessCb, chip::NullOptional);
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

        auto onSuccessCb = [&response](const chip::app::ConcreteCommandPath & commandPath, const chip::app::StatusIB & aStatus,
                                       const typename T::ResponseType & dataResponse) { response = dataResponse; };

        return command_test<T>(sSuite, topic, json_payload, request, onSuccessCb, chip::NullOptional);
    }

    template <typename T>
    inline CHIP_ERROR command_test(nlTestSuite * sSuite, const std::string & topic, const std::string & json_payload,
                                   T & request, uint16_t timedInvokeTimeoutMs)
    {
        auto onSuccessCb = [sSuite](const chip::app::ConcreteCommandPath & commandPath, const chip::app::StatusIB & aStatus,
                                    const chip::app::DataModel::NullObjectType & dataResponse) { NL_TEST_ASSERT(sSuite, true); };

        return command_test<T>(sSuite, topic, json_payload, request, onSuccessCb, chip::MakeOptional(timedInvokeTimeoutMs));
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
