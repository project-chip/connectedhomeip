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
 * @file ota_config.h
 * @brief OTA user configurable settings.
 */

#ifndef OTA_CONFIG_H_
#define OTA_CONFIG_H_

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

/************ End of logging configuration ****************/

/**
 * @brief Log base 2 of the size of the file data block message (excluding the header).
 *
 * 10 bits yields a data block size of 1KB.
 */
#define otaconfigLOG2_FILE_BLOCK_SIZE    10UL


/**
 * @brief Size of the file data block message (excluding the header).
 *
 */
#define otaconfigFILE_BLOCK_SIZE                ( 1UL << otaconfigLOG2_FILE_BLOCK_SIZE )

/**
 * @brief Milliseconds to wait for the self test phase to succeed before we force reset.
 */
#define otaconfigSELF_TEST_RESPONSE_WAIT_MS     16000U

/**
 * @brief Milliseconds to wait before requesting data blocks from the OTA service if nothing is happening.
 *
 * The wait timer is reset whenever a data block is received from the OTA service so we will only send
 * the request message after being idle for this amount of time.
 */
#define otaconfigFILE_REQUEST_WAIT_MS           10000U

/**
 * @brief The maximum allowed length of the thing name used by the OTA agent.
 *
 * AWS IoT requires Thing names to be unique for each device that connects to the broker.
 * Likewise, the OTA agent requires the developer to construct and pass in the Thing name when
 * initializing the OTA agent. The agent uses this size to allocate static storage for the
 * Thing name used in all OTA base topics. Namely $aws/things/<thingName>
 */
#define otaconfigMAX_THINGNAME_LEN              64U

/**
 * @brief The maximum number of data blocks requested from OTA streaming service.
 *
 *  This configuration parameter is sent with data requests and represents the maximum number of
 *  data blocks the service will send in response. The maximum limit for this must be calculated
 *  from the maximum data response limit (128 KB from service) divided by the block size.
 *  For example if block size is set as 1 KB then the maximum number of data blocks that we can
 *  request is 128/1 = 128 blocks. Configure this parameter to this maximum limit or lower based on
 *  how many data blocks response is expected for each data requests.
 *  @note This must be set larger than zero.
 *
 */
#define otaconfigMAX_NUM_BLOCKS_REQUEST         1U

/**
 * @brief The maximum number of requests allowed to send without a response before we abort.
 *
 * This configuration parameter sets the maximum number of times the requests are made over
 * the selected communication channel before aborting and returning error.
 *
 */
#define otaconfigMAX_NUM_REQUEST_MOMENTUM       32U

/**
 * @brief The number of data buffers reserved by the OTA agent.
 *
 * This configurations parameter sets the maximum number of static data buffers used by
 * the OTA agent for job and file data blocks received.
 */
#define otaconfigMAX_NUM_OTA_DATA_BUFFERS       10U

/**
 * @brief How frequently the device will report its OTA progress to the cloud.
 *
 * Device will update the job status with the number of blocks it has received every certain
 * number of blocks it receives. For example, 25 means device will update job status every 25 blocks
 * it receives.
 */
#define otaconfigOTA_UPDATE_STATUS_FREQUENCY    25U

/**
 * @brief Allow update to same or lower version.
 *
 * Set this to 1 to allow downgrade or same version update. This configurations parameter
 * disables version check and allows update to a same or lower version. This is provided for
 * testing purpose and it is recommended to always update to higher version and keep this
 * configuration disabled.
 */
#define otaconfigAllowDowngrade                 0U

/**
 * @brief The protocol selected for OTA control operations.
 *
 * This configurations parameter sets the default protocol for all the OTA control
 * operations like requesting OTA job, updating the job status etc.
 *
 * Note - Only MQTT is supported at this time for control operations.
 */
#define configENABLED_CONTROL_PROTOCOL          ( OTA_CONTROL_OVER_MQTT )

/**
 * @brief The protocol selected for OTA data operations.
 *
 * This configurations parameter sets the protocols selected for the data operations
 * like requesting file blocks from the service.
 *
 * Note - Both MQTT and HTTP is supported for data transfer. This configuration parameter
 * can be set to following -
 * Enable data over MQTT - ( OTA_DATA_OVER_MQTT )
 * Enable data over HTTP - ( OTA_DATA_OVER_HTTP)
 * Enable data over both MQTT & HTTP ( OTA_DATA_OVER_MQTT | OTA_DATA_OVER_HTTP )
 */
#define configENABLED_DATA_PROTOCOLS            ( OTA_DATA_OVER_MQTT )

/**
 * @brief The preferred protocol selected for OTA data operations.
 *
 * Primary data protocol will be the protocol used for downloading file if more than
 * one protocol is selected while creating OTA job. Default primary data protocol is MQTT
 * and following update here to switch to HTTP as primary.
 *
 * Note - use OTA_DATA_OVER_HTTP for HTTP as primary data protocol.
 */
#define configOTA_PRIMARY_DATA_PROTOCOL         ( OTA_DATA_OVER_MQTT )

/**
 * @brief Data type to represent a file.
 *
 * It is used to represent a file received via OTA. The file is declared as
 * the pointer of this type: otaconfigOTA_FILE_TYPE * pFile.
 */
#define otaconfigOTA_FILE_TYPE                  FILE

#endif /* OTA_CONFIG_H_ */
