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
/***************************************************************************//**
 * @file
 * @brief Definitions for the WWAH Server Silabs plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_WWAH_SERVER_SILABS_H
#define SILABS_WWAH_SERVER_SILABS_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

enum {
  PLUGIN_WWAH_CONFIGURATION_MASK_ZLL_POLICY_DISABLED            = 0x01,
  PLUGIN_WWAH_CONFIGURATION_MASK_CONFIGURATION_MODE_ENABLED     = 0x02,
  PLUGIN_WWAH_CONFIGURATION_MASK_PARENT_CLASSIFICATION_ENABLED  = 0x04,
  PLUGIN_WWAH_CONFIGURATION_MASK_DISABLE_OTA_DOWNGRADES         = 0x08
                                                                  // 4 more free bits
};

// The latest WWAH ZCL spec (17-01067-012) defines Parent Classification's
// default to 0x00 but we're working to change it to 0x01 so that first time
// joiners can take advantage of the Parent Classification algorithm
#define PLUGIN_WWAH_CONFIGURATION_MASK_DEFAULT               \
  (PLUGIN_WWAH_CONFIGURATION_MASK_CONFIGURATION_MODE_ENABLED \
   | PLUGIN_WWAH_CONFIGURATION_MASK_DISABLE_OTA_DOWNGRADES)

// Based on "WWAH ZCL Cluster Definition", 4 clusters (OTA, Time, Poll Control, Keep Alive)
// are specified to use ONLY the trust center as the cluster server.
#define MAX_CLUSTER_CAPACITY_FOR_USE_TC_FOR_CLUSTER_SERVER_TOKEN 4

#define PLUGIN_WWAH_USE_TC_FOR_CLUSTER_SERVER_NULL_VALUE 0xFFFF

// Used to handle read attribute response received from the time server.
void emAfPluginSlWwahReadAttributesResponseCallback(EmberAfClusterId clusterId,
                                                    uint8_t *buffer,
                                                    uint16_t bufLen);
#endif

#ifndef EMBER_AF_PLUGIN_WWAH_SERVER_SILABS_SURVEY_BEACON_RESULT_LIMIT
#define EMBER_AF_PLUGIN_WWAH_SERVER_SILABS_SURVEY_BEACON_RESULT_LIMIT 4 // default
#endif

// Status of the Powering Down Notification command that is
// passed into the callback function if not NULL.
typedef enum {
  EMBER_POWER_DOWN_NOTIFICATION_SUCCESS,  // A default response was received with success status.
  EMBER_POWER_DOWN_NOTIFICATION_FAILURE,  // A default response was received with an error status, or other error.
  EMBER_POWER_DOWN_NOTIFICATION_TIMEOUT,  // A default response was not received.
  EMBER_POWER_DOWN_NOTIFICATION_IN_PROGRESS
} EmberPowerDownNotificationResult;

// Send a Powering Off Notification command to the specified destination.
// If the callback function pointer is not NULL, the callback function will
// be called once the Powering Off Notification command is delivered or times out.
void emberAfPluginWwahServerSendPoweringOffNotification(EmberNodeId nodeId,
                                                        uint8_t srcEndpoint,
                                                        uint8_t dstEndpoint,
                                                        EmberAfWwahPowerNotificationReason reason,
                                                        uint16_t manufacturerId,
                                                        uint8_t *manufacturerReason,
                                                        uint8_t manufacturerReasonLen,
                                                        void (*pcallback)(EmberPowerDownNotificationResult) );

// Send a Powering On Notification command to the specified destination.
void emberAfPluginWwahServerSendPoweringOnNotification(EmberNodeId nodeId,
                                                       uint8_t srcEndpoint,
                                                       uint8_t dstEndpoint,
                                                       EmberAfWwahPowerNotificationReason reason,
                                                       uint16_t manufacturerId,
                                                       uint8_t *manufacturerReason,
                                                       uint8_t manufacturerReasonLen);

EmberAfStatus emberReadWwahServerSilabsAttribute(uint8_t endpoint,
                                                 EmberAfAttributeId attributeId,
                                                 const char *name,
                                                 uint8_t *data,
                                                 uint8_t size);

EmberAfStatus emberWriteWwahServerSilabsAttribute(uint8_t endpoint,
                                                  EmberAfAttributeId attributeId,
                                                  const char *name,
                                                  uint8_t *data,
                                                  EmberAfAttributeType type);

bool setupSurveyBeaconProcedure(void);

void sortBeaconSurveyResult(EmberBeaconSurvey surveyResult);

#ifdef EMBER_TEST
EmberMessageBuffer surveyBeaconDataCache;
uint8_t surveyBeaconDataCount;
#endif // EMBER_Test

#endif // SILABS_WWAH_SERVER_SILABS_H
