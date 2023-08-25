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
 * @file ota_demo_core_mqtt.c
 * @brief OTA update example using coreMQTT.
 */

/* Standard includes. */
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

/* Include Demo Config as the first non-system header. */
#include "demo_config.h"
#include "dic.h"
#include "mqtt.h"
#include "MQTT_transport.h"

#include "semphr.h"
#include <string.h>
#include "silabs_utils.h"
#include "ota_cbor_private.h"
/* pthread include. */
/* semaphore include. */
#include "task.h"
/* MQTT include. */
#include "mqtt_subscription_manager.h"

/*Include backoff algorithm header for retry logic.*/
// #include "backoff_algorithm.h"

/* OTA Library include. */
#include "ota.h"
#include "ota_config.h"

/* OTA Library Interface include. */
#include "ota_os_freertos.h"
#include "ota_mqtt_interface.h"
#include "pal.h"

/* Include firmware version struct definition. */
#include "ota_appversion32.h"

/* AWS IoT Core TLS ALPN definitions for MQTT authentication */

/**
 * These configuration settings are required to run the OTA demo which uses mutual authentication.
 * Throw compilation error if the below configs are not defined.
 */
#ifndef AWS_IOT_ENDPOINT
    #error "Please define AWS IoT MQTT broker endpoint(AWS_IOT_ENDPOINT) in demo_config.h."
#endif
#ifndef CLIENT_IDENTIFIER
    #error "Please define a unique client identifier, CLIENT_IDENTIFIER, in demo_config.h."
#endif

/**
 * @brief Length of MQTT server host name.
 */
#define AWS_IOT_ENDPOINT_LENGTH             ( ( uint16_t ) ( sizeof( AWS_IOT_ENDPOINT ) - 1 ) )

/**
 * @brief Length of client identifier.
 */
#define CLIENT_IDENTIFIER_LENGTH            ( ( uint16_t ) ( sizeof( CLIENT_IDENTIFIER ) - 1 ) )

/**
 * @brief Transport timeout in milliseconds for transport send and receive.
 */
#define TRANSPORT_SEND_RECV_TIMEOUT_MS      ( 1000U )

/**
 * @brief Timeout for receiving CONNACK packet in milli seconds.
 */
#define CONNACK_RECV_TIMEOUT_MS             ( 2000U )

/**
 * @brief The maximum time interval in seconds which is allowed to elapse
 * between two Control Packets.
 *
 * It is the responsibility of the Client to ensure that the interval between
 * Control Packets being sent does not exceed the this Keep Alive value. In the
 * absence of sending any other Control Packets, the Client MUST send a
 * PINGREQ Packet.
 */
#define MQTT_KEEP_ALIVE_INTERVAL_SECONDS    ( 60U )

/**
 * @brief Period for waiting on ack.
 */
#define MQTT_ACK_TIMEOUT_MS                 ( 5000U )

/**
 * @brief Period for demo loop sleep in milliseconds.
 */
#define OTA_EXAMPLE_LOOP_SLEEP_PERIOD_MS    ( 50U )

/**
 * @brief Size of the network buffer to receive the MQTT message.
 *
 * The largest message size is data size from the AWS IoT streaming service,
 * otaconfigFILE_BLOCK_SIZE + extra for headers.
 */

#define OTA_NETWORK_BUFFER_SIZE                  ( otaconfigFILE_BLOCK_SIZE + 128 )

/**
 * @brief The delay used in the main OTA Demo task loop to periodically output the OTA
 * statistics like number of packets received, dropped, processed and queued per connection.
 */
#define OTA_EXAMPLE_TASK_DELAY_MS                ( 1000U )

/**
 * @brief The timeout for waiting for the agent to get suspended after closing the
 * connection.
 */
#define OTA_SUSPEND_TIMEOUT_MS                   ( 5000U )

/**
 * @brief The timeout for waiting before exiting the OTA demo.
 */
#define OTA_DEMO_EXIT_TIMEOUT_MS                 ( 3000U )

/**
 * @brief The maximum size of the file paths used in the demo.
 */
#define OTA_MAX_FILE_PATH_SIZE                   ( 260U )

/**
 * @brief The maximum size of the stream name required for downloading update file
 * from streaming service.
 */
#define OTA_MAX_STREAM_NAME_SIZE                 ( 128U )

/**
 * @brief The maximum back-off delay (in milliseconds) for retrying connection to server.
 */
#define CONNECTION_RETRY_MAX_BACKOFF_DELAY_MS    ( 5000U )

/**
 * @brief The base back-off delay (in milliseconds) to use for connection retry attempts.
 */
#define CONNECTION_RETRY_BACKOFF_BASE_MS         ( 500U )

/**
 * @brief Number of milliseconds in a second.
 */
#define NUM_MILLISECONDS_IN_SECOND               ( 1000U )

/**
 * @brief The maximum number of retries for connecting to server.
 */
#define CONNECTION_RETRY_MAX_ATTEMPTS            ( 5U )

/**
 * @brief The MQTT metrics string expected by AWS IoT.
 */
#define METRICS_STRING                           "?SDK=" OS_NAME "&Version=" OS_VERSION "&Platform=" HARDWARE_PLATFORM_NAME "&OTALib=" OTA_LIB

/**
 * @brief The length of the MQTT metrics string expected by AWS IoT.
 */
#define METRICS_STRING_LENGTH                    ( ( uint16_t ) ( sizeof( METRICS_STRING ) - 1 ) )


#ifdef CLIENT_USERNAME

/**
 * @brief Append the username with the metrics string if #CLIENT_USERNAME is defined.
 *
 * This is to support both metrics reporting and username/password based client
 * authentication by AWS IoT.
 */
    #define CLIENT_USERNAME_WITH_METRICS    CLIENT_USERNAME METRICS_STRING
#endif

/**
 * @brief The common prefix for all OTA topics.
 */
#define OTA_TOPIC_PREFIX               "$aws/things/+/"

/**
 * @brief The string used for jobs topics.
 */
#define OTA_TOPIC_JOBS                 "jobs"

/**
 * @brief The string used for streaming service topics.
 */
#define OTA_TOPIC_STREAM               "streams"

/**
 * @brief The length of the outgoing publish records array used by the coreMQTT
 * library to track QoS > 0 packet ACKS for outgoing publishes.
 */
#define OUTGOING_PUBLISH_RECORD_LEN    ( 10U )

/**
 * @brief The length of the incoming publish records array used by the coreMQTT
 * library to track QoS > 0 packet ACKS for incoming publishes.
 */
#define INCOMING_PUBLISH_RECORD_LEN    ( 10U )

/*-----------------------------------------------------------*/

/* Linkage for error reporting. */
extern int errno;

/**
 * @brief Struct for firmware version.
 */
const AppVersion32_t appFirmwareVersion =
{
    .u.x.major = APP_VERSION_MAJOR,
    .u.x.minor = APP_VERSION_MINOR,
    .u.x.build = APP_VERSION_BUILD,
};

extern mqtt_client_t *mqtt_client;

extern MQTT_Transport_t *transport;
/**
 * @brief Keep a flag for indicating if the MQTT connection is alive.
 */
static bool mqttSessionEstablished = false;

/**
 * @brief Structure for openssl parameters.
 */
// static OpensslParams_t opensslParams;

/**
 * @brief Mutex for synchronizing coreMQTT API calls.
 */
  StaticSemaphore_t mqttMutex;
  
  SemaphoreHandle_t sem_mutex = NULL;

/**
 * @brief Semaphore for synchronizing buffer operations.
 */
 StaticSemaphore_t bufferSemaphore;

 SemaphoreHandle_t sem_buffer;

/**
 * @brief Semaphore for synchronizing wait for ack.
 */
  StaticSemaphore_t ackSemaphore;
  
  SemaphoreHandle_t sem_ack;
/**
 * @brief Enum for type of OTA job messages received.
 */
typedef enum jobMessageType
{
    jobMessageTypeNextGetAccepted = 0,
    jobMessageTypeNextNotify,
    jobMessageTypeMax
} jobMessageType_t;

/**
 * @brief The network buffer must remain valid when OTA library task is running.
 */
// static uint8_t otaNetworkBuffer[ OTA_NETWORK_BUFFER_SIZE ];

/**
 * @brief Update File path buffer.
 */
uint8_t updateFilePath[ OTA_MAX_FILE_PATH_SIZE ];

/**
 * @brief Certificate File path buffer.
 */
uint8_t certFilePath[ OTA_MAX_FILE_PATH_SIZE ];

/**
 * @brief Stream name buffer.
 */
uint8_t streamName[ OTA_MAX_STREAM_NAME_SIZE ];

/**
 * @brief Decode memory.
 */
uint8_t decodeMem[ otaconfigFILE_BLOCK_SIZE ];

/**
 * @brief Bitmap memory.
 */
uint8_t bitmap[ OTA_MAX_BLOCK_BITMAP_SIZE ];

/**
 * @brief Event buffer.
 */
static OtaEventData_t eventBuffer[ otaconfigMAX_NUM_OTA_DATA_BUFFERS ];

/**
 * @brief The buffer passed to the OTA Agent from application while initializing.
 */
static OtaAppBuffer_t otaBuffer =
{
    .pUpdateFilePath    = updateFilePath,
    .updateFilePathsize = OTA_MAX_FILE_PATH_SIZE,
    .pCertFilePath      = certFilePath,
    .certFilePathSize   = OTA_MAX_FILE_PATH_SIZE,
    .pStreamName        = streamName,
    .streamNameSize     = OTA_MAX_STREAM_NAME_SIZE,
    .pDecodeMemory      = decodeMem,
    .decodeMemorySize   = otaconfigFILE_BLOCK_SIZE,
    .pFileBitmap        = bitmap,
    .fileBitmapSize     = OTA_MAX_BLOCK_BITMAP_SIZE
};

/**
 * @brief Thread to call the OTA agent task.
 *
 * @param[in] pParam Can be used to pass down functionality to the agent task
 * @return void* returning null.
 */
static void otaThread( void * pParam );

/**
 * @brief Start OTA demo.
 *
 * The OTA task is created with initializing the OTA agent and
 * setting the required interfaces. The demo loop then starts,
 * establishing an MQTT connection with the broker and waiting
 * for an update. After a successful update the OTA agent requests
 * a manual reset to the downloaded executable.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE.
 */
static int startOTADemo( void );

/**
 * @brief Set OTA interfaces.
 *
 * @param[in]  pOtaInterfaces pointer to OTA interface structure.
 */
static void setOtaInterfaces( OtaInterfaces_t * pOtaInterfaces );

/**
 * @brief Disconnect from the MQTT broker and close connection.
 *
 */
// static void disconnect( void );

/**
 * @brief Attempt to connect to the MQTT broker.
 *
 * @return int EXIT_SUCCESS if a connection is established.
 */
static int establishConnection();

/**
 * @brief Initialize MQTT by setting up transport interface and network.
 *
 * @param[in] pMqttContext Structure representing MQTT connection.
 * @param[in] pNetworkContext Network context to connect on.
 * @return int EXIT_SUCCESS if MQTT component is initialized
 */

/**
 * @brief The OTA agent has completed the update job or it is in
 * self test mode. If it was accepted, we want to activate the new image.
 * This typically means we should reset the device to run the new firmware.
 * If now is not a good time to reset the device, it may be activated later
 * by your user code. If the update was rejected, just return without doing
 * anything and we'll wait for another job. If it reported that we should
 * start test mode, normally we would perform some kind of system checks to
 * make sure our new firmware does the basic things we think it should do
 * but we'll just go ahead and set the image as accepted for demo purposes.
 * The accept function varies depending on your platform. Refer to the OTA
 * PAL implementation for your platform in aws_ota_pal.c to see what it
 * does for you.
 *
 * @param[in] event Event from OTA lib of type OtaJobEvent_t.
 * @return None.
 */
static void otaAppCallback( OtaJobEvent_t event,
                            void * pData );

/**
 * @brief callback to use with the MQTT context to notify incoming packet events.
 *
 * @param[in] pMqttContext MQTT context which stores the connection.
 * @param[in] pPacketInfo Parameters of the incoming packet.
 * @param[in] pDeserializedInfo Deserialized packet information to be dispatched by
 * the subscription manager to event callbacks.
 */
static void mqttEventCallback(const char * pTopicFilter,
                                uint16_t topicFilterLength, const void *pPayload, uint16_t payloadLength );

/**
 * @brief Callback registered with the OTA library that notifies the OTA agent
 * of an incoming PUBLISH containing a job document.
 *
 * @param[in] pContext MQTT context which stores the connection.
 * @param[in] pPublishInfo MQTT packet information which stores details of the
 * job document.
 */
static void mqttJobCallback( const char * pTopicName,
                                    uint16_t topicNameLength, const void * pPayload, uint16_t payloadLength );

/**
 * @brief Callback that notifies the OTA library when a data block is received.
 *
 * @param[in] pContext MQTT context which stores the connection.
 * @param[in] pPublishInfo MQTT packet that stores the information of the file block.
 */
static void mqttDataCallback( const char * pTopicName,
                                    uint16_t topicNameLength, const void * pPayload, uint16_t payloadLength );

static SubscriptionManagerCallback_t otaMessageCallback[] = { mqttJobCallback, mqttDataCallback };

/*-----------------------------------------------------------*/

void otaEventBufferFree( OtaEventData_t * const pxBuffer )
{
    if( xSemaphoreTake( sem_buffer, portMAX_DELAY ) == pdTRUE )
    {
        pxBuffer->bufferUsed = false;
        ( void ) xSemaphoreGive( sem_buffer );
    }
    else
    {
        SILABS_LOG("Failed to get buffer semaphore: "
                    ",errno=%s",
                    strerror( errno ) );
    }
}

/*-----------------------------------------------------------*/

OtaEventData_t * otaEventBufferGet( void )
{
    uint32_t ulIndex = 0;
    OtaEventData_t * pFreeBuffer = NULL;
    if( xSemaphoreTake( sem_buffer, portMAX_DELAY) == pdTRUE )
    {
        for( ulIndex = 0; ulIndex < otaconfigMAX_NUM_OTA_DATA_BUFFERS; ulIndex++ )
        {
            if( eventBuffer[ ulIndex ].bufferUsed == false )
            {
                eventBuffer[ ulIndex ].bufferUsed = true;
                pFreeBuffer = &eventBuffer[ ulIndex ];
                break;
            }
        }
        ( void ) xSemaphoreGive( sem_buffer );
    }
    else
    {
        SILABS_LOG("Failed to get buffer semaphore: %s",strerror( errno ));
    }

    return pFreeBuffer;
}

/*-----------------------------------------------------------*/

static void otaAppCallback( OtaJobEvent_t event,
                            void * pData )
{
    OtaErr_t err = OtaErrUninitialized;

    switch( event )
    {
        case OtaJobEventUpdateComplete:
        case OtaJobEventActivate:

            /* Shutdown OTA Agent, if it is required that the unsubscribe operations are not
             * performed while shutting down please set the second parameter to 0 instead of 1. */
            OTA_Shutdown( 0, 1 );

            /* Activate the new firmware image. */
            OTA_ActivateNewImage();
            /* Requires manual activation of new image.*/
            SILABS_LOG("New image activation failed." );

            break;

        case OtaJobEventFail:
            SILABS_LOG("Received OtaJobEventFail callback from OTA Agent.");

            /* Nothing special to do. The OTA agent handles it. */
            break;

        case OtaJobEventStartTest:

            /* This demo just accepts the image since it was a good OTA update and networking
             * and services are all working (or we would not have made it this far). If this
             * were some custom device that wants to test other things before validating new
             * image, this would be the place to kick off those tests before calling
             * OTA_SetImageState() with the final result of either accepted or rejected. */

            err = OTA_SetImageState( OtaImageStateAccepted );

            if( err != OtaErrNone )
            {
                LogError( ( " Failed to set image state as accepted." ) );
            }

            break;

        case OtaJobEventProcessed:

            if( pData != NULL )
            {
                otaEventBufferFree( ( OtaEventData_t * ) pData );
            }

            break;

        case OtaJobEventSelfTestFailed:

            /* Requires manual activation of previous image as self-test for
             * new image downloaded failed.*/

            /* Shutdown OTA Agent, if it is required that the unsubscribe operations are not
             * performed while shutting down please set the second parameter to 0 instead of 1. */
            OTA_Shutdown( 0, 1);

            break;

        default:
            SILABS_LOG("Received invalid callback event from OTA Agent.");
    }
}

jobMessageType_t getJobMessageType( const char * pTopicName,
                                    uint16_t topicNameLength )
{
    uint16_t index = 0U;
    int mqttStatus;
    bool isMatch = false;
    jobMessageType_t jobMessageIndex = jobMessageTypeMax;

    /* For suppressing compiler-warning: unused variable. */
    ( void ) mqttStatus;

    /* Lookup table for OTA job message string. */
    static const char * const pJobTopicFilters[ jobMessageTypeMax ] =
    {
        OTA_TOPIC_PREFIX OTA_TOPIC_JOBS "/$next/get/accepted",
        OTA_TOPIC_PREFIX OTA_TOPIC_JOBS "/notify-next",        
    };
    /* Match the input topic filter against the wild-card pattern of topics filters
    * relevant for the OTA Update service to determine the type of topic filter. */
    for( ; index < jobMessageTypeMax; index++ )
    {
        mqttStatus = MQTT_MatchTopic( pTopicName,
                                      topicNameLength,
                                      pJobTopicFilters[ index ],
                                      strlen( pJobTopicFilters[ index ] ),
                                      &isMatch );
        assert( mqttStatus == 0 );

        if( isMatch )
        {
            jobMessageIndex = index;
            break;
        }
    }
    SILABS_LOG("getJobMessageType jobMessageIndex %d\n", jobMessageIndex);
    return jobMessageIndex;
}

/*-----------------------------------------------------------*/

static void mqttJobCallback( const char * pTopicName,
                                    uint16_t topicNameLength, const void * pPayload, uint16_t payloadLength )
{
    OtaEventData_t * pData;
    OtaEventMsg_t eventMsg = { 0 };
    jobMessageType_t jobMessageType = 0;

    jobMessageType = getJobMessageType( pTopicName, topicNameLength );

    switch( jobMessageType )
    {
        case jobMessageTypeNextGetAccepted:
        case jobMessageTypeNextNotify:

            pData = otaEventBufferGet();

            if( pData != NULL )
            {
                memcpy( pData->data, pPayload, payloadLength );
                pData->dataLength = payloadLength;
                eventMsg.eventId = OtaAgentEventReceivedJobDocument;
                eventMsg.pEventData = pData;
                /* Send job document received event. */
                OTA_SignalEvent( &eventMsg );
            }
            else
            {
                SILABS_LOG("No OTA data buffers available." );
            }

            break;

        default:
            SILABS_LOG("Received job message %s size %ld.\n\n",
                       pTopicName,
                       payloadLength );
    }
}

/*-----------------------------------------------------------*/

static void mqttDataCallback( const char * pTopicName, uint16_t topicNameLength, const void * pPayload, uint16_t payloadLength )
{
    OtaEventData_t * pData;
    OtaEventMsg_t eventMsg = { 0 };

    (void) pTopicName;
    (void) topicNameLength;

    pData = otaEventBufferGet();

    if( pData != NULL )
    {
        memcpy( pData->data, pPayload, payloadLength );
        pData->dataLength = payloadLength;
        eventMsg.eventId = OtaAgentEventReceivedFileBlock;
        eventMsg.pEventData = pData;

        /* Send job document received event. */
        OTA_SignalEvent( &eventMsg );
    }
    else
    {
        SILABS_LOG( "No OTA data buffers available." );
    }

}
/*-----------------------------------------------------------*/

static void mqttEventCallback( const char * pTopicFilter,
                                uint16_t topicFilterLength, const void *pPayload, uint16_t payloadLength )
{
    /* Handle incoming publish. The lower 4 bits of the publish packet
     * type is used for the dup, QoS, and retain flags. Hence masking
     * out the lower bits to check if the packet is publish. */
    /* Handle incoming publish. */
    assert( pTopicFilter != NULL );
    assert( topicFilterLength != 0 );
    assert( pPayload != NULL );
    assert( payloadLength != 0);
    SubscriptionManager_DispatchHandler(  pTopicFilter, topicFilterLength , pPayload, payloadLength );

}

static int establishConnection( void )
{
    int returnStatus = EXIT_FAILURE;

    /* Set the pParams member of the network context with desired transport. */
    // networkContext.pParams = &opensslParams;

    /* Attempt to connect to the MQTT broker. If connection fails, retry after
     * a timeout. Timeout value will be exponentially increased till the maximum
     * attempts are reached or maximum timeout value is reached. The function
     * returns EXIT_FAILURE if the TCP connection cannot be established to
     * broker after configured number of attempts. */
    if(dic_init_status())
    {
        mqttSessionEstablished = true;
        returnStatus = EXIT_SUCCESS;
        SILABS_LOG("DIC Init is completed");
    }
    
    return returnStatus;
}

static void registerSubscriptionManagerCallback( const char * pTopicFilter,
                                                 uint16_t topicFilterLength )
{
    bool isMatch = false;
    int mqttStatus;
    SubscriptionManagerStatus_t subscriptionStatus = SUBSCRIPTION_MANAGER_SUCCESS;

    uint16_t index = 0U;

    /* For suppressing compiler-warning: unused variable. */
    ( void ) mqttStatus;

    /* Lookup table for OTA message string. */
    static const char * const pWildCardTopicFilters[] =
    {
        OTA_TOPIC_PREFIX OTA_TOPIC_JOBS "/#",
        OTA_TOPIC_PREFIX OTA_TOPIC_STREAM "/#"
    };

    /* Match the input topic filter against the wild-card pattern of topics filters
    * relevant for the OTA Update service to determine the type of topic filter. */
    for( ; index < 2; index++ )
    {
        mqttStatus = MQTT_MatchTopic( pTopicFilter,
                                      topicFilterLength,
                                      pWildCardTopicFilters[ index ],
                                      strlen( pWildCardTopicFilters[ index ] ),
                                      &isMatch );
        assert( mqttStatus == 0 );

        if( isMatch )
        {
            /* Register callback to subscription manager. */
            subscriptionStatus = SubscriptionManager_RegisterCallback( pWildCardTopicFilters[ index ],
                                                                       strlen( pWildCardTopicFilters[ index ] ),
                                                                       otaMessageCallback[ index ] );

            if( subscriptionStatus != SUBSCRIPTION_MANAGER_SUCCESS )
            {
                SILABS_LOG("Failed to register a callback to subscription manager with error = %d.",
                           subscriptionStatus );
            }

            break;
        }
    }
}

/*-----------------------------------------------------------*/

static OtaMqttStatus_t mqttSubscribe( const char * pTopicFilter,
                                      uint16_t topicFilterLength,
                                      uint8_t qos )
{
    OtaMqttStatus_t otaRet = OtaMqttSuccess;

    int mqttStatus;
    
    assert( pTopicFilter != NULL );
    assert( topicFilterLength > 0 );

    ( void ) qos;

    if( xSemaphoreTake( sem_mutex, portMAX_DELAY ) == pdTRUE )
    {
        mqttStatus = dic_aws_ota_subscribe(pTopicFilter, qos, mqttEventCallback);

        xSemaphoreGive( sem_mutex );
    }
    else
    {
        SILABS_LOG("Failed to acquire mqtt mutex for executing MQTT_Subscribe"
                    ",errno=%s",
                    strerror( errno ) );
    }

    if( mqttStatus != 0 )
    {
        otaRet = OtaMqttSubscribeFailed;
    }
    else
    {
        SILABS_LOG("SUBSCRIBE topic %.*s to broker.\n\n",
                   topicFilterLength,
                   pTopicFilter );

        registerSubscriptionManagerCallback( pTopicFilter, topicFilterLength );
    }

    return otaRet;
}

/*-----------------------------------------------------------*/

static OtaMqttStatus_t mqttPublish( const char * const pacTopic,
                                    uint16_t topicLen,
                                    const char * pMsg,
                                    uint32_t msgSize,
                                    uint8_t qos )
{
    OtaMqttStatus_t otaRet = OtaMqttSuccess;

    int mqttStatus;

    if( xSemaphoreTake( sem_mutex, portMAX_DELAY ) == pdTRUE )
    {
        mqttStatus = dic_aws_ota_publish(pacTopic, pMsg, msgSize, qos);
        if( mqttStatus != 0 )
        {
            otaRet = OtaMqttPublishFailed;
        }

        xSemaphoreGive( sem_mutex );
    }
    else
    {
        SILABS_LOG("Failed to acquire mqtt mutex for executing MQTT_Publish"
                    ",errno=%s",
                    strerror( errno ));

        otaRet = OtaMqttPublishFailed;
    }

    if( ( mqttStatus == 0 ) && ( qos == 1 ) )
    {
        BaseType_t ret;

        ret = xSemaphoreTake( sem_ack, portMAX_DELAY );

        if( ret != pdTRUE )
        {
            otaRet = OtaMqttPublishFailed;
        }else{
            SILABS_LOG("receive ack for publish, ret=%ld", ret);
        }

        if( ret == -1 )
        {
            SILABS_LOG("Failed to receive ack for publish."
                        ",errno=%s",
                        strerror( errno ));

            otaRet = OtaMqttPublishFailed;
        }
        xSemaphoreGive( sem_ack );
    }

    return otaRet;
}

/*-----------------------------------------------------------*/

static OtaMqttStatus_t mqttUnsubscribe( const char * pTopicFilter,
                                        uint16_t topicFilterLength,
                                        uint8_t qos )
{
    OtaMqttStatus_t otaRet = OtaMqttSuccess;
    int mqttStatus;

    ( void ) qos;

    if( xSemaphoreTake( sem_mutex, portMAX_DELAY ) == pdTRUE )
    {
        mqttStatus = dic_aws_ota_unsubscribe( pTopicFilter);
        xSemaphoreGive( sem_mutex );
    }
    else
    {
        SILABS_LOG( "Failed to acquire mutex for executing MQTT_Unsubscribe"
                    ",errno=%s",
                    strerror( errno ) );
    }

    if( mqttStatus != 0 )
    {
        otaRet = OtaMqttUnsubscribeFailed;
    }
    else
    {
        SILABS_LOG( "UNSUBSCRIBE topic %.*s to broker.\n\n",
                   topicFilterLength,
                   pTopicFilter );
    }

    return otaRet;
}

/*-----------------------------------------------------------*/

static void setOtaInterfaces( OtaInterfaces_t * pOtaInterfaces )
{
    /* Initialize OTA library OS Interface. */
    pOtaInterfaces->os.event.init = OtaInitEvent_FreeRTOS;
    pOtaInterfaces->os.event.send = OtaSendEvent_FreeRTOS;
    pOtaInterfaces->os.event.recv = OtaReceiveEvent_FreeRTOS;
    pOtaInterfaces->os.event.deinit = OtaDeinitEvent_FreeRTOS;
    pOtaInterfaces->os.timer.start = OtaStartTimer_FreeRTOS;
    pOtaInterfaces->os.timer.stop = OtaStopTimer_FreeRTOS;
    pOtaInterfaces->os.timer.delete = OtaDeleteTimer_FreeRTOS;
    pOtaInterfaces->os.mem.malloc = Malloc_FreeRTOS;
    pOtaInterfaces->os.mem.free = Free_FreeRTOS;

    /* Initialize the OTA library MQTT Interface.*/
    pOtaInterfaces->mqtt.subscribe = mqttSubscribe;
    pOtaInterfaces->mqtt.publish = mqttPublish;
    pOtaInterfaces->mqtt.unsubscribe = mqttUnsubscribe;

    /* Initialize the OTA library PAL Interface.*/
    pOtaInterfaces->pal.getPlatformImageState = otaPal_GetPlatformImageState;
    pOtaInterfaces->pal.setPlatformImageState = otaPal_SetPlatformImageState;
    pOtaInterfaces->pal.writeBlock = otaPal_WriteBlock;
    pOtaInterfaces->pal.activate = otaPal_ActivateNewImage;
    pOtaInterfaces->pal.closeFile = otaPal_CloseFile;
    pOtaInterfaces->pal.reset = otaPal_ResetDevice;
    pOtaInterfaces->pal.abort = otaPal_Abort;
    pOtaInterfaces->pal.createFile = otaPal_CreateFileForRx;
}

/*-----------------------------------------------------------*/

static void otaThread( void * pParam )
{
    /* Calling OTA agent task. */
    OTA_EventProcessingTask( pParam );
    SILABS_LOG("===================================OTA Agent stopped.===================================" );
    return;
}
/*-----------------------------------------------------------*/
static int startOTADemo( void )
{
    /* Status indicating a successful demo or not. */
    int returnStatus = EXIT_SUCCESS;

    /* coreMQTT library return status. */
    int mqttStatus;

    /* OTA library return status. */
    OtaErr_t otaRet = OtaErrNone;

    /* OTA Agent state returned from calling OTA_GetAgentState.*/
    OtaState_t state = OtaAgentStateStopped;

    /* OTA event message used for sending event to OTA Agent.*/
    OtaEventMsg_t eventMsg = { 0 };

    /* OTA library packet statistics per job.*/
    OtaAgentStatistics_t otaStatistics = { 0 };

    static TaskHandle_t OTAThreadTask;

    /* OTA interface context required for library interface functions.*/
    OtaInterfaces_t otaInterfaces;

    /* Maximum time to wait for the OTA agent to get suspended. */
    int16_t suspendTimeout;

    /* Set OTA Library interfaces.*/
    setOtaInterfaces( &otaInterfaces );

    SILABS_LOG("OTA over MQTT demo, Application version %u.%u.%u",
               appFirmwareVersion.u.x.major,
               appFirmwareVersion.u.x.minor,
               appFirmwareVersion.u.x.build );

    /****************************** Init OTA Library. ******************************/

    if( returnStatus == EXIT_SUCCESS )
    {
        if( ( otaRet = OTA_Init( &otaBuffer,
                                 &otaInterfaces,
                                 ( const uint8_t * ) ( CLIENT_IDENTIFIER ),
                                 otaAppCallback ) ) != OtaErrNone )
        {
            returnStatus = EXIT_FAILURE;
        }
        SILABS_LOG("OTA Init is Successfull");
    } 

    /****************************** Create OTA Task. ******************************/

    if( returnStatus == EXIT_SUCCESS )
    {
        if((pdPASS != xTaskCreate(otaThread, "OTA_THREAD", OTA_TASK_STACK_SIZE, NULL, OTA_TASK_PRIORITY, &OTAThreadTask )) || !OTAThreadTask)
        {
            returnStatus = EXIT_FAILURE;
        }
        SILABS_LOG("Task creation successfull for OTA thread");    
    }

    /****************************** OTA Demo loop. ******************************/

    if( returnStatus == EXIT_SUCCESS )
    {
        /* Wait till OTA library is stopped, output statistics for currently running
         * OTA job */
        while( ( ( state = OTA_GetState() ) != OtaAgentStateStopped ) )
        {
            if( mqttSessionEstablished != true )
            {
                /* Connect to MQTT broker and create MQTT connection. */
                if( EXIT_SUCCESS == establishConnection() )
                {
                    /* Check if OTA process was suspended and resume if required. */
                    if( state == OtaAgentStateSuspended )
                    {
                        /* Resume OTA operations. */
                        OTA_Resume();
                    }
                    else
                    {
                        /* Send start event to OTA Agent.*/
                        eventMsg.eventId = OtaAgentEventStart;
                        OTA_SignalEvent( &eventMsg );
                    }
                }
            }
            if( mqttSessionEstablished == true )
            {
                if( ( mqttStatus == 0 ) )
                {
                    /* Get OTA statistics for currently executing job. */
                    OTA_GetStatistics( &otaStatistics );
                    /* Delay to allow data to buffer for MQTT_ProcessLoop. */
                    vTaskDelay( OTA_EXAMPLE_LOOP_SLEEP_PERIOD_MS );
                }
                else
                {
                    /* Suspend OTA operations. */
                    otaRet = OTA_Suspend();

                    if( otaRet == OtaErrNone )
                    {
                        suspendTimeout = OTA_SUSPEND_TIMEOUT_MS;

                        while( ( ( state = OTA_GetState() ) != OtaAgentStateSuspended ) && ( suspendTimeout > 0 ) )
                        {
                            /* Wait for OTA Library state to suspend */
                            vTaskDelay( OTA_EXAMPLE_TASK_DELAY_MS );
                            suspendTimeout -= OTA_EXAMPLE_TASK_DELAY_MS;
                        }
                    }
                    else
                    {
                        SILABS_LOG( "OTA failed to suspend. "
                                    "StatusCode=%d.", otaRet );
                    }
                }
            }
        }
    }

    /****************************** Wait for OTA Thread. ******************************/
    if( returnStatus == EXIT_SUCCESS )
    {
        eTaskState task_state = eTaskGetState(OTAThreadTask);

    switch(task_state){
        case eReady:
            break;
        case eRunning:
            break;
        case eBlocked:
            break;
        case eSuspended:
            break;
        case eDeleted:
            vTaskDelete(OTAThreadTask);
            returnStatus = EXIT_FAILURE;
            break;
        default:
            break;
        }
    }

    return returnStatus;
}

/*-----------------------------------------------------------*/

/**
 * @brief Entry point of demo.
 *
 * This example initializes the OTA library to enable OTA updates via the
 * MQTT broker. It simply connects to the MQTT broker with the users
 * credentials and spins in an indefinite loop to allow MQTT messages to be
 * forwarded to the OTA agent for possible processing. The OTA agent does all
 * of the real work; checking to see if the message topic is one destined for
 * the OTA agent. If not, it is simply ignored.
 */
int aws_ota_init( void* parameters )
{
    ( void ) parameters;
    // ( void ) argv;

    /* Return error status. */
    int returnStatus = EXIT_SUCCESS;

    /* Semaphore initialization flag. */
    bool bufferSemInitialized = false;
    bool ackSemInitialized = false;
    bool mqttMutexInitialized = false;

    /* Maximum time in milliseconds to wait before exiting demo . */
    int16_t waitTimeoutMs = OTA_DEMO_EXIT_TIMEOUT_MS;

    /* Initialize semaphore for buffer operations. */
    // bufferSemaphore = xSemaphoreCreateCounting(1,0);
    sem_buffer = xSemaphoreCreateCountingStatic(1, 1, &bufferSemaphore);
    if( sem_buffer == NULL )
    {
        SILABS_LOG("Failed to initialize buffer semaphore");
        returnStatus = EXIT_FAILURE;
    }
    else
    {
        bufferSemInitialized = true;
    }

    /* Initialize semaphore for ack. */
    sem_ack = xSemaphoreCreateCountingStatic(1, 1, &ackSemaphore);

    if( sem_ack == NULL )
    {
        SILABS_LOG( "Failed to initialize acksem semaphore" );
        returnStatus = EXIT_FAILURE;
    }
    else
    {
        ackSemInitialized = true;
    }

    /* Initialize mutex for coreMQTT APIs. */
    sem_mutex = xSemaphoreCreateMutexStatic( &mqttMutex );

    if(sem_mutex == NULL)
    {
        SILABS_LOG( "Failed to initialize mutex semaphore" );
        returnStatus = EXIT_FAILURE;
    }
    else
    {
        mqttMutexInitialized = true;
    }

    if( returnStatus == EXIT_SUCCESS )
    {
        /* Start OTA demo. */
        returnStatus = startOTADemo();
    }

    /* Disconnect from broker and close connection. */
    dic_aws_ota_close();

    if( bufferSemInitialized == true )
    {   
        /* Cleanup semaphore created for buffer operations. */
        vSemaphoreDelete( sem_buffer );
        if( sem_buffer != NULL )
        {
            returnStatus = EXIT_FAILURE;
        }
    }

    if( ackSemInitialized == true )
    {
        /* Cleanup semaphore created for ack. */
        vSemaphoreDelete( sem_ack );
        if( sem_ack != NULL )
        {
            returnStatus = EXIT_FAILURE;
        }
    }

    if( mqttMutexInitialized == true )
    {
        /* Cleanup mutex created for MQTT operations. */
        vSemaphoreDelete( sem_mutex );
        if( sem_mutex != NULL )
        {
            returnStatus = EXIT_FAILURE;
        }
    }

    /* Wait and log message before exiting demo. */
    while( waitTimeoutMs > 0 )
    {
        vTaskDelay( OTA_EXAMPLE_TASK_DELAY_MS );
        waitTimeoutMs -= OTA_EXAMPLE_TASK_DELAY_MS;
    }

    return returnStatus;
}