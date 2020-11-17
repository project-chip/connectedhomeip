/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *
 *    Copyright (c) 2020 Silicon Labs
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
/****************************************************************************
 * @file
 * @brief This is the source for the plugin used to
 *add an IAS Zone cluster server to a project. This
 *source handles zone enrollment and storing of
 * attributes from a CIE device and provides an API
 *for different plugins to post updated zone status
 *values.
 *******************************************************************************
 ******************************************************************************/

#pragma once

#include <app/util/af-types.h>

#define EM_AF_UNKNOWN_ENDPOINT 0
// Absolute max backoff time, at least one retry a day
// (ie. 24 hours * 60 minutes * 60 seconds).
#define IAS_ZONE_STATUS_QUEUE_RETRY_ABS_MAX_BACKOFF_TIME_SEC (24 * 60 * 60)

// Definitions for the IAS Zone enrollment mode.
typedef enum
{
    EMBER_ZCL_IAS_ZONE_ENROLLMENT_MODE_TRIP_TO_PAIR             = 0x00,
    EMBER_ZCL_IAS_ZONE_ENROLLMENT_MODE_AUTO_ENROLLMENT_RESPONSE = 0x01,
    EMBER_ZCL_IAS_ZONE_ENROLLMENT_MODE_REQUEST                  = 0x02
} EmberAfIasZoneEnrollmentMode;

// Status queue retry parameters
typedef struct
{
    uint8_t firstBackoffTimeSec;
    uint8_t backoffSeqCommonRatio;
    uint32_t maxBackoffTimeSec;
    bool unlimitedRetries;
    uint8_t maxRetryAttempts;
} IasZoneStatusQueueRetryConfig;

/** @brief Updates the zone status for an endpoint.
 *
 * This function will update the zone status attribute of the specified endpoint
 * using the specified new zone status. It will then notify the CIE of the
 * updated status.
 *
 * @param endpoint The endpoint whose zone status attribute is to be updated.
 * @param newStatus The new status to write to the attribute.
 * @param timeSinceStatusOccurredQs The amount of time (in quarter seconds) that
 *   has passed since the status change occurred.
 *
 * @return EMBER_SUCCESS if the attribute update and notify succeeded, error
 * code otherwise.
 */
EmberStatus emberAfPluginIasZoneServerUpdateZoneStatus(chip::EndpointId endpoint, uint16_t newStatus,
                                                       uint16_t timeSinceStatusOccurredQs);

/** @brief Gets the CIE assigned zone id of a given endpoint.
 *
 * This function will return the zone ID that was assigned to the given
 * endpoint by the CIE at time of enrollment.
 *
 * @param endpoint The endpoint whose ID is to be queried.
 *
 * @return The zone ID assigned by the CIE at time of enrollment.
 */
uint8_t emberAfPluginIasZoneServerGetZoneId(chip::EndpointId endpoint);

/** @brief Determines the enrollment status of a given endpoint.
 *
 * This function will return true or false depending on whether the specified
 * endpoint has undergone IAS Zone Enrollment.
 *
 * @param endpoint The endpoint whose enrollment status is to be queried.
 *
 * @return True if enrolled, false otherwise.
 */
bool emberAfIasZoneClusterAmIEnrolled(chip::EndpointId endpoint);

/** @brief Set the enrollment status.
 *
 * This function will return the status of the set enrollment method attempt.
 *
 * @param endpoint The endpoint whose enrollment method is to be set
 * @param method The enrollment method that should be set
 *
 * @return An ::EmberAfStatus value indicating the status of the set action.
 */
EmberAfStatus emberAfPluginIasZoneClusterSetEnrollmentMethod(chip::EndpointId endpoint, EmberAfIasZoneEnrollmentMode method);

/** @brief Configure the retry parameters of the status queue.
 *
 * This function will configure the status queue retry parameters.
 *
 * @param retryConfig Status queue retry configuration.
 */
EmberStatus emberAfIasZoneServerConfigStatusQueueRetryParams(IasZoneStatusQueueRetryConfig * retryConfig);

/** @brief Set the retry parameters of the status queue to default.
 *
 * This function will set the status queue retry parameters to their default values.
 */
void emberAfIasZoneServerSetStatusQueueRetryParamsToDefault(void);

/** @brief Discards any pendint events in the status queue and sets it inactive.
 *
 * This function will discard any pending event pending in the status queue.
 * Also, the status queue event control manager will be inactivated.
 */
void emberAfIasZoneServerDiscardPendingEventsInStatusQueue(void);

/** @brief Prints information on the satus queue.
 */
void emberAfPluginIasZoneServerPrintQueue(void);

/** @brief Prints the satus queue config.
 */
void emberAfPluginIasZoneServerPrintQueueConfig(void);
