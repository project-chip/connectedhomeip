/*
 * AWS IoT Device SDK for Embedded C 202211.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file mqtt_subscription_manager.h
 * @brief The API of a subscription manager for handling subscription callbacks
 * to topic filters in MQTT operations.
 */

#ifndef MQTT_SUBSCRIPTION_MANAGER_H_
#define MQTT_SUBSCRIPTION_MANAGER_H_

/**************************************************/
/******* DO NOT CHANGE the following order ********/
/**************************************************/

/* Logging related header files are required to be included in the following order:
 * 1. Include the header file "logging_levels.h".
 * 2. Define LIBRARY_LOG_NAME and  LIBRARY_LOG_LEVEL.
 * 3. Include the header file "logging_stack.h".
 */

/* Include header that defines log levels. */
// #include "logging_levels.h"

/* Logging configuration for the Subscription Manager module. */
#ifndef LIBRARY_LOG_NAME
    #define LIBRARY_LOG_NAME     "Subscription Manager"
#endif
#ifndef LIBRARY_LOG_LEVEL
    #define LIBRARY_LOG_LEVEL    LOG_DEBUG
#endif

// #include "logging_stack.h"

/************ End of logging configuration ****************/

/* Include MQTT library. */
#include "mqtt.h"
#include "stdbool.h"

/* Enumeration type for return status value from Subscription Manager API. */
typedef enum SubscriptionManagerStatus
{
    /**
     * @brief Success return value from Subscription Manager API.
     */
    SUBSCRIPTION_MANAGER_SUCCESS = 1,

    /**
     * @brief Failure return value due to registry being full.
     */
    SUBSCRIPTION_MANAGER_REGISTRY_FULL = 2,

    /**
     * @brief Failure return value due to an already existing record in the
     * registry for a new callback registration's requested topic filter.
     */
    SUBSCRIPTION_MANAGER_RECORD_EXISTS = 3
} SubscriptionManagerStatus_t;


/**
 * @brief Callback type to be registered for a topic filter with the subscription manager.
 *
 * For incoming PUBLISH messages received on topics that match the registered topic filter,
 * the callback would be invoked by the subscription manager.
 *
 * @param[in] pContext The context associated with the MQTT connection.
 * @param[in] pPublishInfo The incoming PUBLISH message information.
 */
typedef void (* SubscriptionManagerCallback_t )( const char * pTopicName,
                                    uint16_t topicNameLength, const void * pPayload, uint16_t payloadLength );

/**
 * @brief Dispatches the incoming PUBLISH message to the callbacks that have their
 * registered topic filters matching the incoming PUBLISH topic name. The dispatch
 * handler will invoke all these callbacks with matching topic filters.
 *
 * @param[in] pContext The context associated with the MQTT connection.
 * @param[in] pPublishInfo The incoming PUBLISH message information.
 */
void SubscriptionManager_DispatchHandler( const char * pTopicFilter,
                                          uint16_t topicFilterLength,const void *pPayload, uint16_t payloadLength );

/**
 * @brief Utility to register a callback for a topic filter in the subscription manager.
 *
 * The callback will be invoked when an incoming PUBLISH message is received on
 * a topic that matches the topic filter, @a pTopicFilter. The subscription manager
 * accepts wildcard topic filters.
 *
 * @param[in] pTopicFilter The topic filter to register the callback for.
 * @param[in] topicFilterLength The length of the topic filter string.
 * @param[in] callback The callback to be registered for the topic filter.
 *
 * @note The subscription manager does not allow more than one callback to be registered
 * for the same topic filter.
 * @note The passed topic filter, @a pTopicFilter, is saved in the registry.
 * The application must not free or alter the content of the topic filter memory
 * until the callback for the topic filter is removed from the subscription manager.
 *
 * @return Returns one of the following:
 * - #SUBSCRIPTION_MANAGER_SUCCESS if registration of the callback is successful.
 * - #SUBSCRIPTION_MANAGER_REGISTRY_FULL if the registration failed due to registry
 * being already full.
 * - #SUBSCRIPTION_MANAGER_RECORD_EXISTS, if a registered callback already exists for
 * the requested topic filter in the subscription manager.
 */
SubscriptionManagerStatus_t SubscriptionManager_RegisterCallback( const char * pTopicFilter,
                                                                  uint16_t topicFilterLength,
                                                                  SubscriptionManagerCallback_t pCallback );

/**
 * @brief Utility to remove the callback registered for a topic filter from the
 * subscription manager.
 *
 * @param[in] pTopicFilter The topic filter to remove from the subscription manager.
 * @param[in] topicFilterLength The length of the topic filter string.
 */
void SubscriptionManager_RemoveCallback( const char * pTopicFilter,
                                         uint16_t topicFilterLength );


int MQTT_MatchTopic( const char * pTopicName,
                              const uint16_t topicNameLength,
                              const char * pTopicFilter,
                              const uint16_t topicFilterLength,
                              bool * pIsMatch );

#endif /* ifndef MQTT_SUBSCRIPTION_MANAGER_H_ */