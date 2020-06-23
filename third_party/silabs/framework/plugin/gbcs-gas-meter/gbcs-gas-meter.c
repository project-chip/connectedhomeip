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
/***************************************************************************//**
 * @file
 * @brief Routines for the GBCS Gas Meter plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "app/framework/plugin/tunneling-client/tunneling-client.h"
#include "app/framework/plugin/gbcs-device-log/gbcs-device-log.h"
#include "app/framework/plugin/gbz-message-controller/gbz-message-controller.h"
#include "gbcs-gas-meter.h"

// Plugin configuration options

// This is a local endpoint representing the GSME.
#define GSME_ENDPOINT EMBER_AF_PLUGIN_GBCS_GAS_METER_GSME_ENDPOINT
// This is the remote endpoint representing the Remote CommsHub which is part of the CHF.
#define CHF_ENDPOINT  EMBER_AF_PLUGIN_GBCS_GAS_METER_CHF_ENDPOINT
// Offset from the begin of a tunneled remote party message where the GBZ packet begins
#define GBZ_OFFSET EMBER_AF_PLUGIN_GBCS_GAS_METER_GBZ_OFFSET
// The GBCS spec recommends this value at 30 minutes (1800 seconds).
// For testing purposes we default to 20 seconds.
#define MIRROR_UPDATE_INTERVAL_SECONDS EMBER_AF_PLUGIN_GBCS_GAS_METER_REPORT_INTERVAL

// This enum is used by the state machine to keep track of what
// the sleepy gas meter is doing.
enum {
  INITIAL_STATE,
  REGISTRATION_COMPLETE,
  SERVICE_DISCOVERY_STARTED,
  MIRROR_CONFIGURATION_STARTED,
  MIRROR_READY,
  MIRROR_UPDATE_IN_PROGRESS
};
static uint8_t state;

// Sleepy meter events, this event is included in the application event
// configuration run by the application framework. The application framework
// will allow the device to sleep until this event or others need to fire.
// This event replaces the use of the emberAfMainTickCallback which should
// not be used by sleepy devices as it does not properly allow for power
// management.
EmberEventControl emberAfPluginGbcsGasMeterSleepyMeterEventControl;

/*
 * Per section 10.2.2 of the GBCS version 0.8
 *
 * "Devices shall set the value of the ManufacturerCode field in any
 * RequestTunnel command to 0xFFFF ('not used').
 *
 * The ProtocolID of all Remote Party Messages shall be 6 ('GB-HGRP'). Devices
 * shall set the value of the ProtocolID field in any RequestTunnel command to 6.
 *
 * Devices shall set the value of the FlowControlSupport field in any
 * RequestTunnel command to 'False'.
 */
#define GBCS_TUNNELING_MANUFACTURER_CODE      0xFFFF
#define GBCS_TUNNELING_PROTOCOL_ID            0x06
#define GBCS_TUNNELING_FLOW_CONTROL_SUPPORT   false

/*
 * This enum defines the state associated with the tunnel to the CHF
 */
typedef enum {
  UNUSED_TUNNEL,
  REQUEST_PENDING_TUNNEL,
  RESPONSE_PENDING_TUNNEL,
  ACTIVE_TUNNEL,
  CLOSED_TUNNEL
} EmAfGSMETunnelState;

typedef struct {
  uint8_t remoteEndpoint;
  EmberNodeId remoteNodeId;
  EmAfGSMETunnelState state;
  uint8_t tunnelId;
} EmAfGSMETunnel;

static EmAfGSMETunnel tunnel;

// Tunnel Manager Header values per GNBCS spec
#define TUNNEL_MANAGER_HEADER_GET          0x01
#define TUNNEL_MANAGER_HEADER_GET_RESPONSE 0x02
#define TUNNEL_MANAGER_HEADER_PUT          0x03

/*
 * Temporary storage for tunnel messages where the data includes a header
 */
#define TEMP_MESSAGE_LEN 1500
static uint8_t message[TEMP_MESSAGE_LEN];

// Number of seconds before trying to find a mirror
// Once connected to a network and SE registration is complete,
// the sleepy gas meter will try to find a mirror every x seconds.
#define INITIAL_STATE_EVENT_INTERVAL_SECONDS 10

// Global used to track mirror endpoint
static uint8_t mirrorEndpoint = 0xFF;
static uint16_t mirrorAddress = 0;

// Number of quarter seconds between segments of the mirror
// update. During an update of the mirror, the sleepy gas meter
// will send out update segments every x quarter seconds.
#define MIRROR_UPDATE_IN_PROGRESS_INTERVAL_QUARTER_SECONDS 2

typedef struct {
  bool report;
  EmberAfClusterId clusterId;
  uint16_t attributeCount;
  EmberAfAttributeId *attributeIds;
} EmAfSGSMEAttributeData;

static EmberAfAttributeId basicClusterAttributes[] = {
  ZCL_VERSION_ATTRIBUTE_ID,
  ZCL_MODEL_IDENTIFIER_ATTRIBUTE_ID,
  ZCL_MANUFACTURER_NAME_ATTRIBUTE_ID,
  ZCL_HW_VERSION_ATTRIBUTE_ID,
  ZCL_POWER_SOURCE_ATTRIBUTE_ID,
};
#define GAS_METER_BASIC_CLUSTER_ATTRIBUTE_COUNT ((sizeof(basicClusterAttributes) / sizeof(basicClusterAttributes[0])))

static EmberAfAttributeId meteringClusterAttributes[] = {
  ZCL_CURRENT_SUMMATION_DELIVERED_ATTRIBUTE_ID,
  ZCL_SUPPLY_STATUS_ATTRIBUTE_ID,
  ZCL_CURRENT_TIER1_SUMMATION_DELIVERED_ATTRIBUTE_ID,
  ZCL_CURRENT_TIER2_SUMMATION_DELIVERED_ATTRIBUTE_ID,
  ZCL_CURRENT_TIER3_SUMMATION_DELIVERED_ATTRIBUTE_ID,
  ZCL_CURRENT_TIER4_SUMMATION_DELIVERED_ATTRIBUTE_ID,
  ZCL_STATUS_ATTRIBUTE_ID,
  ZCL_REMAINING_BATTERY_LIFE_IN_DAYS_ATTRIBUTE_ID,
  ZCL_CURRENT_METER_ID_ATTRIBUTE_ID,
  ZCL_UNIT_OF_MEASURE_ATTRIBUTE_ID,
  ZCL_MULTIPLIER_ATTRIBUTE_ID,
  ZCL_DIVISOR_ATTRIBUTE_ID,
  ZCL_SUMMATION_FORMATTING_ATTRIBUTE_ID,
  ZCL_METERING_DEVICE_TYPE_ATTRIBUTE_ID,
  ZCL_SITE_ID_ATTRIBUTE_ID,
  ZCL_CUSTOMER_ID_NUMBER_ATTRIBUTE_ID,
  ZCL_ALTERNATIVE_UNIT_OF_MEASURE_ATTRIBUTE_ID,
  ZCL_ALTERNATIVE_CONSUMPTION_FORMATTING_ATTRIBUTE_ID,
  ZCL_SUPPLY_TAMPER_STATE_ATTRIBUTE_ID,
  ZCL_SUPPLY_DEPLETION_STATE_ATTRIBUTE_ID,
  ZCL_CURRENT_NO_TIER_BLOCK1_SUMMATION_DELIVERED_ATTRIBUTE_ID,
  ZCL_CURRENT_NO_TIER_BLOCK2_SUMMATION_DELIVERED_ATTRIBUTE_ID,
  ZCL_CURRENT_NO_TIER_BLOCK3_SUMMATION_DELIVERED_ATTRIBUTE_ID,
  ZCL_CURRENT_NO_TIER_BLOCK4_SUMMATION_DELIVERED_ATTRIBUTE_ID,
  ZCL_BILL_TO_DATE_DELIVERED_ATTRIBUTE_ID,
  ZCL_BILL_DELIVERED_TRAILING_DIGIT_ATTRIBUTE_ID,
  ZCL_CURRENT_ALTERNATIVE_DAY_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY2_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY3_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY4_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY5_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY6_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY7_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY8_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_CURRENT_WEEK_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_WEEK_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_WEEK2_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_WEEK3_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_WEEK4_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_WEEK5_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_CURRENT_MONTH_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH2_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH3_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH4_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH5_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH6_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH7_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH8_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH9_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH10_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH11_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH12_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH13_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_SIMPLE_METERING_CLUSTER_REPORTING_STATUS_SERVER_ATTRIBUTE_ID
};
#define GAS_METER_METERING_CLUSTER_ATTRIBUTE_COUNT ((sizeof(meteringClusterAttributes) / sizeof(meteringClusterAttributes[0])))

static EmberAfAttributeId prepaymentClusterAttributes[] = {
  ZCL_PAYMENT_CONTROL_CONFIGURATION_ATTRIBUTE_ID,
  ZCL_CREDIT_REMAINING_ATTRIBUTE_ID,
  ZCL_EMERGENCY_CREDIT_REMAINING_ATTRIBUTE_ID,
  ZCL_ACCUMULATED_DEBT_ATTRIBUTE_ID,
  ZCL_OVERALL_DEBT_CAP_ATTRIBUTE_ID,
  ZCL_EMERGENCY_CREDIT_LIMIT_ALLOWANCE_ATTRIBUTE_ID,
  ZCL_EMERGENCY_CREDIT_THRESHOLD_ATTRIBUTE_ID,
  ZCL_MAX_CREDIT_LIMIT_ATTRIBUTE_ID,
  ZCL_MAX_CREDIT_PER_TOP_UP_ATTRIBUTE_ID,
  ZCL_LOW_CREDIT_WARNING_ATTRIBUTE_ID,
  ZCL_CUT_OFF_VALUE_ATTRIBUTE_ID,
  ZCL_DEBT_AMOUNT_1_ATTRIBUTE_ID,
  ZCL_DEBT_RECOVERY_FREQUENCY_1_ATTRIBUTE_ID,
  ZCL_DEBT_RECOVERY_AMOUNT_1_ATTRIBUTE_ID,
  ZCL_DEBT_AMOUNT_2_ATTRIBUTE_ID,
  ZCL_DEBT_RECOVERY_FREQUENCY_2_ATTRIBUTE_ID,
  ZCL_DEBT_RECOVERY_AMOUNT_2_ATTRIBUTE_ID,
  ZCL_DEBT_AMOUNT_3_ATTRIBUTE_ID,
  ZCL_DEBT_RECOVERY_TOP_UP_PERCENTAGE_3_ATTRIBUTE_ID,
  ZCL_PREPAYMENT_ALARM_STATUS_ATTRIBUTE_ID,
  ZCL_HISTORICAL_COST_CONSUMPTION_FORMATTING_ATTRIBUTE_ID,
  ZCL_CONSUMPTION_UNIT_OF_MEASUREMENT_ATTRIBUTE_ID,
  ZCL_CURRENCY_SCALING_FACTOR_ATTRIBUTE_ID,
  ZCL_PREPAYMANT_CURRENCY_ATTRIBUTE_ID,
  ZCL_CURRENT_DAY_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY_2_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY_3_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY_4_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY_5_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY_6_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY_7_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY_8_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_CURRENT_WEEK_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_WEEK_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_WEEK_2_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_WEEK_3_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_WEEK_4_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_WEEK_5_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_CURRENT_MONTH_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH_2_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH_3_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH_4_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH_5_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH_6_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH_7_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH_8_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH_9_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH_10_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH_11_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH_12_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH_13_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
};
#define GAS_METER_PREPAYMENT_CLUSTER_ATTRIBUTE_COUNT ((sizeof(prepaymentClusterAttributes) / sizeof(prepaymentClusterAttributes[0])))

static EmAfSGSMEAttributeData reportList[] = {
  { false, ZCL_BASIC_CLUSTER_ID, GAS_METER_BASIC_CLUSTER_ATTRIBUTE_COUNT, basicClusterAttributes },
  { false, ZCL_PREPAYMENT_CLUSTER_ID, GAS_METER_PREPAYMENT_CLUSTER_ATTRIBUTE_COUNT, prepaymentClusterAttributes },
  { false, ZCL_SIMPLE_METERING_CLUSTER_ID, GAS_METER_METERING_CLUSTER_ATTRIBUTE_COUNT, meteringClusterAttributes },
};
#define GAS_METER_REPORT_LIST_COUNT ((sizeof(reportList) / sizeof(reportList[0])))
#define GAS_METER_REPORT_LIST_BASIC_CLUSTER_INDEX       0
#define GAS_METER_REPORT_LIST_METERING_CLUSTER_INDEX    1
#define GAS_METER_REPORT_LIST_PREPAYMENT_CLUSTER_INDEX  2
#define GAS_METER_REPORT_LIST_PREPAYMENT_CLUSTER_INDEX  2
#define GAS_METER_REPORT_LIST_OPERATIONAL_INDEX         3

#define GAS_METER_ATTRIBUTE_RECORD_BUFFER_SIZE 70
static uint8_t data[GAS_METER_ATTRIBUTE_RECORD_BUFFER_SIZE];
static uint8_t clusterToReport = 0;
static uint8_t attributeToReport = 0;

//------------------------------------------------------------------------------
// Forward Declarations

static void setSleepyMeterState(uint8_t newState);
static void serviceDiscoveryCallback(const EmberAfServiceDiscoveryResult* result);
static bool tunnelCreate(EmberNodeId remoteNodeId,
                         uint8_t remoteEndpoint);
static bool tunnelDestroy(void);
static bool tunnelSendData(uint16_t dataLen,
                           uint8_t *data);
static bool requestTunnel(void);
static bool handleRequestTunnelFailure(EmberAfPluginTunnelingClientStatus status);
static void handleRemotePartyMessage(uint16_t gbzCommandLen, uint8_t *gbzCommand);

//------------------------------------------------------------------------------
// External Functions

/** @brief Report Attributes
 *
 * If the mirror is ready to receive attribute reports and there is not already
 * a report in progress the this function will kick off the task of reporting
 * all attributes to the mirror.
 */
void emberAfPluginGbcsGasMeterReportAttributes(void)
{
  uint8_t i;
  uint8_t reportingStatus = EMBER_ZCL_ATTRIBUTE_REPORTING_STATUS_ATTRIBUTE_REPORTING_COMPLETE;

  if (state == MIRROR_READY) {
    for (i = 0; i < GAS_METER_REPORT_LIST_COUNT; i++) {
      reportList[i].report = true;
    }
    emberAfWriteServerAttribute(GSME_ENDPOINT,
                                ZCL_SIMPLE_METERING_CLUSTER_ID,
                                ZCL_SIMPLE_METERING_CLUSTER_REPORTING_STATUS_SERVER_ATTRIBUTE_ID,
                                &reportingStatus,
                                ZCL_ENUM8_ATTRIBUTE_TYPE);
    setSleepyMeterState(MIRROR_UPDATE_IN_PROGRESS);
  } else {
    emberAfPluginGbcsGasMeterPrintln("Cannot report attributes. State is 0x%x", state);
  }
}

//------------------------------------------------------------------------------
// Callbacks

/** @brief Plugin Init
 *
 * This function is called from the application's main function. It gives the
 * application a chance to do any initialization required at system startup. Any
 * code that you would normally put into the top of the application's main()
 * routine should be put into this function.
        Note: No callback in the
 * Application Framework is associated with resource cleanup. If you are
 * implementing your application on a Unix host where resource cleanup is a
 * consideration, we expect that you will use the standard Posix system calls,
 * including the use of atexit() and handlers for signals such as SIGTERM,
 * SIGINT, SIGCHLD, SIGPIPE and so on. If you use the signal() function to
 * register your signal handler, please mind the returned value which may be an
 * Application Framework function. If the return value is non-null, please make
 * sure that you call the returned function from your handler to avoid negating
 * the resource cleanup of the Application Framework itself.
 *
 */
void emberAfPluginGbcsGasMeterInitCallback(void)
{
  tunnel.state = UNUSED_TUNNEL;
  setSleepyMeterState(INITIAL_STATE);
}

// *******************************************************************
// * clusterInitCallback
// *
// * Make sure that the simple metering device type is set to gas meter.
// *
// *******************************************************************
void emberAfClusterInitCallback(uint8_t endpointId,
                                uint16_t clusterId)
{
  uint8_t deviceType = EMBER_ZCL_METERING_DEVICE_TYPE_GAS_METERING;
  if ((endpointId == GSME_ENDPOINT)
      && (clusterId == ZCL_SIMPLE_METERING_CLUSTER_ID)) {
    emberAfWriteAttribute(endpointId,
                          ZCL_SIMPLE_METERING_CLUSTER_ID,
                          ZCL_METERING_DEVICE_TYPE_ATTRIBUTE_ID,
                          CLUSTER_MASK_SERVER,
                          (uint8_t*) &deviceType,
                          ZCL_INT8U_ATTRIBUTE_TYPE);
  }
}

// *******************************************************************
// * sleepyMeterEventCallback
// *
// * State machine for discovering a mirror, configuring the mirror, and
// * periodically reporting attributes.
// *
// *******************************************************************
void emberAfPluginGbcsGasMeterSleepyMeterEventHandler(void)
{
  EmberStatus status;
  EmberNodeId partnerShortId = 0x0000; // Node ID of trust center which is the Comms Hub
  EmberEUI64 partnerEui;

  // Polling the network state every tick is ineffecient and does not allow the device
  // to sleep.  So let's create a little delay.
  emberAfEventControlSetDelay(&emberAfPluginGbcsGasMeterSleepyMeterEventControl,
                              2 * MILLISECOND_TICKS_PER_SECOND);

  // don't do anything unless we're on the network
  if (emberNetworkState() != EMBER_JOINED_NETWORK) {
    setSleepyMeterState(INITIAL_STATE);
    return;
  }

  switch (state) {
    case INITIAL_STATE:
      // Nothing to do until SE registration is complete
      if (tunnel.state != UNUSED_TUNNEL) {
        tunnelDestroy();
      }
      break;

    case REGISTRATION_COMPLETE:
      // From GBCS v0.8 Section 10.2.2.2
      //
      // When a GSME has successfully established a shared secret key using CBKE
      // with a Communications Hub, the GSME shall:
      //  *  send a request to the ZigBee Gas ESI Endpoint requesting the creation
      //     of a mirrored Metering Cluster using the RequestMirror command;
      //  *  configure the ZigBee Gas Mirror Endpoint to use the two way mirroring
      //     notification scheme 'Scheme B' ; and
      //  *  send a RequestTunnel command to the CHF to request a tunnel association
      //     with the CHF.
      //
      // For clarity, the GSME:
      //  *  shall have access to the Notification Flags on the Communications Hub
      //     whenever it can communicate with the Communications Hub; and
      //  *  shall not provide any metering data to the ZigBee Gas Mirror Endpoint
      //     until and unless the GPF's Entity Identifier is recorded in the GSME
      //     Device Log.

      // We're only giving the tunnel creation a best effort at link key establishment
      // stage.  If it fails for any reason we'll just wait until the app
      // attempts to send data to try the tunnel creation again.
      if (EMBER_SUCCESS == emberLookupEui64ByNodeId(partnerShortId, partnerEui)
          && emberAfPluginGbcsDeviceLogExists(partnerEui, EMBER_AF_GBCS_CHF_DEVICE_TYPE)) {
        tunnelCreate(partnerShortId, CHF_ENDPOINT);
      }

      // We'll look for all devices that support the metering client cluster. Once a device
      // responds we'll check their physical environment attribute to see if they
      // support mirroring.
      status = emberAfFindDevicesByProfileAndCluster(EMBER_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS,
                                                     SE_PROFILE_ID,
                                                     ZCL_SIMPLE_METERING_CLUSTER_ID,
                                                     EMBER_AF_CLIENT_CLUSTER_DISCOVERY,
                                                     serviceDiscoveryCallback);
      if (status == EMBER_SUCCESS) {
        setSleepyMeterState(SERVICE_DISCOVERY_STARTED);
      } else {
        setSleepyMeterState(INITIAL_STATE);
      }
      break;

    case MIRROR_READY:
      // Once a mirror has been found and configured we go into a loop of updating
      // it every time this event is called but first check if we need to retry the
      // tunnel creation.
      if (tunnel.state == REQUEST_PENDING_TUNNEL) {
        emberAfPluginGbcsGasMeterPrintln("Retrying tunnel creation to node ID 0x%2x",
                                         tunnel.remoteNodeId);
        requestTunnel();
      }

      // Report all attributes.  TODO: we probably should only report those
      // attributes that have changed but since this is a test app we'll just send
      // them all.
      emberAfPluginGbcsGasMeterReportAttributes();
      break;

    case MIRROR_UPDATE_IN_PROGRESS:
      while (clusterToReport < GAS_METER_REPORT_LIST_COUNT) {
        if (reportList[clusterToReport].report) {
          uint8_t bufIndex = 0;

          while (attributeToReport < reportList[clusterToReport].attributeCount) {
            EmberAfStatus status = emberAfAppendAttributeReportFields(GSME_ENDPOINT,
                                                                      reportList[clusterToReport].clusterId,
                                                                      reportList[clusterToReport].attributeIds[attributeToReport],
                                                                      CLUSTER_MASK_SERVER,
                                                                      data,
                                                                      GAS_METER_ATTRIBUTE_RECORD_BUFFER_SIZE,
                                                                      &bufIndex);
            if (status == EMBER_ZCL_STATUS_INSUFFICIENT_SPACE) {
              break;
            }
            attributeToReport++;
          }
          emberAfFillCommandGlobalServerToClientReportAttributes(reportList[clusterToReport].clusterId,
                                                                 data,
                                                                 bufIndex);
          emberAfSetCommandEndpoints(GSME_ENDPOINT, mirrorEndpoint);
          emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, mirrorAddress);
          if (attributeToReport >= reportList[clusterToReport].attributeCount) {
            reportList[clusterToReport].report = false;
            attributeToReport = 0;
            clusterToReport++;
          }
          break;
        }
        clusterToReport++;
      }

      if (clusterToReport < GAS_METER_REPORT_LIST_COUNT) {
        setSleepyMeterState(MIRROR_UPDATE_IN_PROGRESS);
      } else {
        uint8_t reportingStatus = EMBER_ZCL_ATTRIBUTE_REPORTING_STATUS_PENDING;
        // Reset the last att written, this will start writing
        // of attributes to the mirror until they are all written.
        clusterToReport = 0;
        emberAfWriteServerAttribute(GSME_ENDPOINT,
                                    ZCL_SIMPLE_METERING_CLUSTER_ID,
                                    ZCL_SIMPLE_METERING_CLUSTER_REPORTING_STATUS_SERVER_ATTRIBUTE_ID,
                                    &reportingStatus,
                                    ZCL_ENUM8_ATTRIBUTE_TYPE);
        setSleepyMeterState(MIRROR_READY);
      }
      break;

    case SERVICE_DISCOVERY_STARTED:
    case MIRROR_CONFIGURATION_STARTED:
    // If we are in service discovery or mirror configuration we are looking for
    // a meter mirror and once found configuring it and as such we are
    // essentially in a holding pattern.
    default:
      setSleepyMeterState(state); //maintain current state
  }
}

/** @brief Read Attributes Response
 *
 * This function is called by the application framework when a Read Attributes
 * Response command is received from an external device.  The application should
 * return true if the message was processed or false if it was not.
 *
 * @param clusterId The cluster identifier of this response.  Ver.: always
 * @param buffer Buffer containing the list of read attribute status records.
 * Ver.: always
 * @param bufLen The length in bytes of the list.  Ver.: always
 */
bool emberAfReadAttributesResponseCallback(EmberAfClusterId clusterId,
                                           uint8_t * buffer,
                                           uint16_t bufLen)
{
  // * we read the physical environment during mirror discovery, if we find
  // * a device on which the physical environment is set, we send a mirror
  // * request.
  if ((state == SERVICE_DISCOVERY_STARTED || state == MIRROR_CONFIGURATION_STARTED)
      && clusterId == ZCL_BASIC_CLUSTER_ID) {
    if (bufLen < 5) {
      return false;
    }
    if ((emberAfGetInt16u(buffer, 0, bufLen)
         == ZCL_PHYSICAL_ENVIRONMENT_ATTRIBUTE_ID)
        && (emberAfGetInt8u(buffer, 2, bufLen) == 0)
        // TODO: currently if we reset the GSME node we loose knowledge of the
        // mirror because that data is not persisted.  In that case the remote
        // physical environment attribute read will not indicate that it can
        // support a mirror because that node was not reset.  So for now
        // we just ignore the value reported in the attribute but later when
        // we have a solution for persistence we should re-enable the following
        // check.
        /* && (emberAfGetInt8u(buffer, 4, bufLen) & 0x01) */) {
      EmberAfClusterCommand *currentCommand = emberAfCurrentCommand();
      emberAfFillCommandSimpleMeteringClusterRequestMirror();
      emberAfSetCommandEndpoints(GSME_ENDPOINT,
                                 currentCommand->apsFrame->sourceEndpoint);
      emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, currentCommand->source);
      setSleepyMeterState(MIRROR_CONFIGURATION_STARTED);
    }
  }
  return false;
}

void emberAfPluginSimpleMeteringServerProcessNotificationFlagsCallback(EmberAfAttributeId attributeId,
                                                                       uint32_t bitMap)
{
  uint32_t requestMessages = 0;

  emberAfPluginGbcsGasMeterPrintln("GSME: ProcessNotificationFlags: 0x%2x, 0x%4x",
                                   attributeId, bitMap);

  /*
   * From GBCS
   *
   * For clarity, the GSME:
   *
   *  - shall not action ZSE / ZCL commands received from the GPF in relation
   *    to any of the flags within NotificationFlags2, NotificationFlags3 and
   *    NotificationFlags5;
   *
   *  - for NotificationFlags4, shall only action ZSE / ZCL commands received
   *    from the GPF in relation to the flags specified below.
   *
   *      Bit Number    Waiting Command
   *      6             Get Prepay Snapshot
   *      7             Get Top Up Log
   *      9             Get Debt Repayment Log
   *
   *  - for FunctionalNotificationFlags, shall only action ZSE / ZCL commands
   *    received from the GPF in relation to the flags specified below
   *
   *      Bit Number   Waiting Command
   *      0            New OTA Firmware
   *      1            CBKE Update Request
   *      4            Stay Awake Request HAN
   *      5            Stay Awake Request WAN
   *      6-8          Push Historical Metering Data Attribute Set
   *      9-11         Push Historical Prepayment Data Attribute Set
   *      12           Push All Static Data - Basic Cluster
   *      13           Push All Static Data - Metering Cluster
   *      14           Push All Static Data - Prepayment Cluster
   *      15           NetworkKeyActive
   *      21           Tunnel Message Pending
   *      22           GetSnapshot
   *      23           GetSampledData
   */

  if (state == MIRROR_READY || state == MIRROR_UPDATE_IN_PROGRESS) {
    if (attributeId == ZCL_NOTIFICATION_FLAGS_4_ATTRIBUTE_ID) {
      requestMessages = bitMap & (EMBER_AF_METERING_NF4_GET_PREPAY_SNAPSHOT | EMBER_AF_METERING_NF4_GET_TOP_UP_LOG | EMBER_AF_METERING_NF4_GET_DEBT_REPAYMENT_LOG);
    } else if (attributeId == ZCL_FUNCTIONAL_NOTIFICATION_FLAGS_ATTRIBUTE_ID) {
      if (bitMap & EMBER_AF_METERING_FNF_NEW_OTA_FIRMWARE) {
        // TODO
      }
      if (bitMap & EMBER_AF_METERING_FNF_CBKE_UPDATE_REQUEST) {
        // TODO
      }
      if (bitMap & (EMBER_AF_METERING_FNF_STAY_AWAKE_REQUEST_HAN | EMBER_AF_METERING_FNF_STAY_AWAKE_REQUEST_WAN)) {
        // TODO
      }
      if (bitMap & EMBER_AF_METERING_FNF_PUSH_HISTORICAL_METERING_DATA_ATTRIBUTE_SET) {
        // TODO - we really should be just sending a subset of the metering attributes
        // but since this is just a test app we'll send them all
        reportList[GAS_METER_REPORT_LIST_METERING_CLUSTER_INDEX].report = true;
        setSleepyMeterState(MIRROR_UPDATE_IN_PROGRESS);
      }
      if (bitMap & EMBER_AF_METERING_FNF_PUSH_HISTORICAL_PREPAYMENT_DATA_ATTRIBUTE_SET) {
        // TODO - we really should be just sending a subset of the prepayment attributes
        // but since this is just a test app we'll send them all
        reportList[GAS_METER_REPORT_LIST_PREPAYMENT_CLUSTER_INDEX].report = true;
        setSleepyMeterState(MIRROR_UPDATE_IN_PROGRESS);
      }
      if (bitMap & EMBER_AF_METERING_FNF_PUSH_ALL_STATIC_DATA_BASIC_CLUSTER) {
        reportList[GAS_METER_REPORT_LIST_BASIC_CLUSTER_INDEX].report = true;
        setSleepyMeterState(MIRROR_UPDATE_IN_PROGRESS);
      }
      if (bitMap & EMBER_AF_METERING_FNF_PUSH_ALL_STATIC_DATA_METERING_CLUSTER) {
        reportList[GAS_METER_REPORT_LIST_METERING_CLUSTER_INDEX].report = true;
        setSleepyMeterState(MIRROR_UPDATE_IN_PROGRESS);
      }
      if (bitMap & EMBER_AF_METERING_FNF_PUSH_ALL_STATIC_DATA_PREPAYMENT_CLUSTER) {
        reportList[GAS_METER_REPORT_LIST_PREPAYMENT_CLUSTER_INDEX].report = true;
        setSleepyMeterState(MIRROR_UPDATE_IN_PROGRESS);
      }
      if (bitMap & EMBER_AF_METERING_FNF_NETWORK_KEY_ACTIVE) {
        // TODO
      }
      if (bitMap & EMBER_AF_METERING_FNF_TUNNEL_MESSAGE_PENDING) {
        // There's tunnel data for us so send the "GET" message
        uint8_t header = TUNNEL_MANAGER_HEADER_GET;
        tunnelSendData(1, &header);
      }
      requestMessages = bitMap & (EMBER_AF_METERING_FNF_GET_SNAPSHOT | EMBER_AF_METERING_FNF_GET_SAMPLED_DATA);
    }
    if (requestMessages != 0) {
      emberAfPluginGbcsGasMeterPrintln("GSME: GetNotifiedMessage: 0x%2x, 0x%4x",
                                       attributeId, requestMessages);
      emberAfFillCommandSimpleMeteringClusterGetNotifiedMessage(EMBER_ZCL_NOTIFICATION_SCHEME_PREDEFINED_NOTIFICATION_SCHEME_B,
                                                                attributeId,
                                                                requestMessages);
      emberAfSetCommandEndpoints(GSME_ENDPOINT, mirrorEndpoint);
      emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, mirrorAddress);
    }
  }
}

/** @brief Request Mirror Response
 *
 * @param endpointId   Ver.: always
 */
bool emberAfSimpleMeteringClusterRequestMirrorResponseCallback(uint16_t endpointId)
{
  if (endpointId == 0xffff) {
    emberAfAppPrintln("Mirror add FAILED");
  } else {
    if (state != MIRROR_READY) {
      mirrorEndpoint = endpointId;
      mirrorAddress = emberAfCurrentCommand()->source;
      emberAfAppPrintln("Mirror ADDED on 0x%2x, 0x%x", mirrorAddress, endpointId);

      uint32_t issuerEventId = emberAfGetCurrentTime();
      emberAfFillCommandSimpleMeteringClusterConfigureMirror(issuerEventId,
                                                             MIRROR_UPDATE_INTERVAL_SECONDS,
                                                             true,
                                                             EMBER_ZCL_NOTIFICATION_SCHEME_PREDEFINED_NOTIFICATION_SCHEME_B);
      emberAfSetCommandEndpoints(GSME_ENDPOINT, mirrorEndpoint);
      emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, mirrorAddress);

      setSleepyMeterState(MIRROR_READY);
    } else {
      emberAfAppPrintln("Mirror add for 0x%2x, 0x%x ignored, already mirrored on 0x%2x 0x%x.",
                        emberAfCurrentCommand()->source, endpointId,
                        mirrorAddress, mirrorEndpoint);
    }
  }
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}

/** @brief Mirror Removed
 *
 * @param endpointId   Ver.: always
 */
bool emberAfSimpleMeteringClusterMirrorRemovedCallback(uint16_t endpointId)
{
  // * This callback simply prints out the endpoint from which
  // * the mirror was removed, and sets our state back to looking for
  // * a new mirror
  if (endpointId == 0xffff) {
    emberAfAppPrintln("Mirror remove FAILED");
  } else {
    emberAfAppPrintln("Mirror REMOVED from %x", endpointId);
    setSleepyMeterState(INITIAL_STATE);
  }
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}

/** @brief Registration
 *
 * This callback is called when the device joins a network and the process of
 * registration is complete. This callback provides a success value of true if
 * the registration process was successful and a value of false if registration
 * failed.
 *
 * @param success true if registration succeeded, false otherwise.  Ver.: always
 */
void emberAfRegistrationCallback(bool success)
{
  emberAfPluginGbcsGasMeterPrintln("GSME: Smart Energy Registration %p.",
                                   (success ? "completed" : "FAILED"));
  if (!success) {
    return;
  }

  setSleepyMeterState(REGISTRATION_COMPLETE);
}

/** @brief Tunnel Opened
 *
 * This function is called by the Tunneling client plugin whenever a tunnel is
 * opened.
 *
 * @param tunnelId The ID of the tunnel that has been opened.  Ver.:
 * always
 * @param tunnelStatus The status of the request.  Ver.: always
 * @param maximumIncomingTransferSize The maximum incoming transfer size of
 * the server.  Ver.: always
 */
void emberAfPluginTunnelingClientTunnelOpenedCallback(uint8_t tunnelId,
                                                      EmberAfPluginTunnelingClientStatus tunnelStatus,
                                                      uint16_t maximumIncomingTransferSize)
{
  emberAfPluginGbcsGasMeterPrintln("GSME: ClientTunnelOpened:0x%x,0x%x,0x%2x",
                                   tunnelId, tunnelStatus, maximumIncomingTransferSize);

  if (tunnelStatus == EMBER_AF_PLUGIN_TUNNELING_CLIENT_SUCCESS) {
    tunnel.tunnelId = tunnelId;
    tunnel.state = ACTIVE_TUNNEL;

    // Per GBCS v0.8 section 10.2.2, Devices supporting the Tunneling Cluster
    // as a Server shall have a MaximumIncomingTransferSize set to 1500 octets,
    // in line with the ZSE default.  All Devices supporting the Tunneling
    // Cluster shall use this value in any RequestTunnelResponse command and
    // any RequestTunnel command.
    //
    // So rather than bring down the tunnel in the case when the maximumIncomingTransferSize
    // is less than 1500 we'll just log a warning message.
    if (maximumIncomingTransferSize < 1500) {
      emberAfAppPrintln("Warning: tunnel opened but MaximumIncomingTransferSize of server is %d but should be 1500",
                        maximumIncomingTransferSize);
    }
    return;
  }

  // see if we can recover from the open failure
  handleRequestTunnelFailure(tunnelStatus);
}

/** @brief Data Received
 *
 * This function is called by the Tunneling client plugin whenever data is
 * received from a server through a tunnel.
 *
 * @param tunnelId The id of the tunnel through which the data was
 * received.  Ver.: always
 * @param data Buffer containing the raw octets of the data.  Ver.: always
 * @param dataLen The length in octets of the data.  Ver.: always
 */
void emberAfPluginTunnelingClientDataReceivedCallback(uint8_t tunnelId,
                                                      uint8_t *data,
                                                      uint16_t dataLen)
{
  emberAfPluginGbcsGasMeterPrint("GSME: ClientDataReceived:%x,[", tunnelId);
  emberAfPluginGbcsGasMeterPrintBuffer(data, dataLen, false);
  emberAfPluginGbcsGasMeterPrintln("]");

  if (tunnel.tunnelId != tunnelId || dataLen < 2
      || *data != TUNNEL_MANAGER_HEADER_GET_RESPONSE) {
    return;
  }

  // GET-RESPONSE (the concatenation 0x02 || number of Remote Party Messages remaining):
  // this is used by the CHF to send a Remote Party Message to the GSME. It also indicates
  // how many Remote Party Messages have yet to be retrieved;
  //
  handleRemotePartyMessage(dataLen - 2, data + 2);

  // If there are more messages to read then ask for them now
  if (*(data + 1) > 0) {
    uint8_t header = TUNNEL_MANAGER_HEADER_GET;
    tunnelSendData(1, &header);
  }
}

/** @brief Tunnel Closed
 *
 * This function is called by the Tunneling client plugin whenever a server
 * sends a notification that it preemptively closed an inactive tunnel.
 * Servers are not required to notify clients of tunnel closures, so
 * applications cannot rely on this callback being called for all tunnels.
 *
 * @param tunnelId The ID of the tunnel that has been closed.  Ver.:
 * always
 */
void emberAfPluginTunnelingClientTunnelClosedCallback(uint8_t tunnelId)
{
  emberAfPluginGbcsGasMeterPrintln("GSME: ClientTunnelClosed:0x%x", tunnelId);

  if (tunnel.tunnelId == tunnelId) {
    tunnel.state = CLOSED_TUNNEL;
  }
}

/** @brief Device Removed
 *
 * This callback is called by the plugin when a device is removed from the
 * device log.
 *
 * @param deviceId Identifier of the device removed  Ver.: always
 */
void emberAfPluginGbcsDeviceLogDeviceRemovedCallback(EmberEUI64 deviceId)
{
  emberAfPluginGbcsGasMeterPrint("GSME: DeviceLogDeviceRemoved ");
  emberAfPluginGbcsGasMeterDebugExec(emberAfPrintBigEndianEui64(deviceId));
  emberAfPluginGbcsGasMeterPrintln("");

  tunnelDestroy();
}

//------------------------------------------------------------------------------
// Internal Functions

// A function used to set the current state of the sleepy meter
// and schedule the timing of the next sleepy meter event. Each call to the
// sleepy meter event should result in updating of the state and thus the
// scheduling of the next event.
static void setSleepyMeterState(uint8_t newState)
{
  state = newState;
  switch (state) {
    case INITIAL_STATE:
    case REGISTRATION_COMPLETE:
      emberEventControlSetDelayQS(emberAfPluginGbcsGasMeterSleepyMeterEventControl, (INITIAL_STATE_EVENT_INTERVAL_SECONDS << 2));
      break;
    case MIRROR_READY:
      emberEventControlSetDelayQS(emberAfPluginGbcsGasMeterSleepyMeterEventControl, (MIRROR_UPDATE_INTERVAL_SECONDS << 2));
      break;
    case SERVICE_DISCOVERY_STARTED:
    case MIRROR_CONFIGURATION_STARTED:
    case MIRROR_UPDATE_IN_PROGRESS:
    default:
      emberEventControlSetDelayQS(emberAfPluginGbcsGasMeterSleepyMeterEventControl, MIRROR_UPDATE_IN_PROGRESS_INTERVAL_QUARTER_SECONDS);
      break;
  }
}

static bool tunnelCreate(EmberNodeId remoteNodeId,
                         uint8_t remoteEndpoint)
{
  emberAfPluginGbcsGasMeterPrintln("GSME: TunnelCreate 0x%2x 0x%x",
                                   remoteNodeId, remoteEndpoint);

  // We only support one tunnel to a given node so if we already have a tunnel
  // lets work with it.
  if (tunnel.state != UNUSED_TUNNEL) {
    if (tunnel.remoteNodeId == remoteNodeId) {
      if (tunnel.state == CLOSED_TUNNEL) {
        return requestTunnel();
      }
    } else {
      tunnelDestroy();
    }
  }

  tunnel.remoteNodeId = remoteNodeId;
  tunnel.remoteEndpoint = remoteEndpoint;
  tunnel.state = CLOSED_TUNNEL;
  tunnel.tunnelId = 0xFF;
  return requestTunnel();
}

static bool tunnelDestroy(void)
{
  emberAfPluginGbcsGasMeterPrintln("GSME: TunnelDestroy");

  EmberAfStatus status = EMBER_ZCL_STATUS_NOT_FOUND;
  if (tunnel.state != UNUSED_TUNNEL) {
    status = emberAfPluginTunnelingClientCloseTunnel(tunnel.tunnelId);
    if (status == EMBER_ZCL_STATUS_SUCCESS || status == EMBER_ZCL_STATUS_NOT_FOUND) {
      tunnel.state = UNUSED_TUNNEL;
      status = EMBER_ZCL_STATUS_SUCCESS;
    }
  }
  return (status == EMBER_ZCL_STATUS_SUCCESS);
}

static bool tunnelSendData(uint16_t dataLen,
                           uint8_t *data)
{
  EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;
  bool success;

  emberAfPluginGbcsGasMeterPrint("GSME: TunnelSendData [");
  emberAfPluginGbcsGasMeterPrintBuffer(data, dataLen, false);
  emberAfPluginGbcsGasMeterPrintln("]");

  if (tunnel.state == ACTIVE_TUNNEL) {
    status = emberAfPluginTunnelingClientTransferData(tunnel.tunnelId, data, dataLen);
  } else if (tunnel.state == CLOSED_TUNNEL) {
    // we'll return failure to this message but we'll start the process
    // of bring up the tunnel so that if the message is resent the tunnel
    // should be up.
    requestTunnel();
  }

  success = (status == EMBER_ZCL_STATUS_SUCCESS);
  if (!success) {
    emberAfAppPrintln("%p%p%p0x%x",
                      "Error: ",
                      "Tunnel SendData failed: ",
                      "Tunneling Status: ",
                      status);
  }
  return success;
}

static bool requestTunnel(void)
{
  EmberAfPluginTunnelingClientStatus status;

  status = emberAfPluginTunnelingClientRequestTunnel(tunnel.remoteNodeId,
                                                     GSME_ENDPOINT,
                                                     tunnel.remoteEndpoint,
                                                     GBCS_TUNNELING_PROTOCOL_ID,
                                                     GBCS_TUNNELING_MANUFACTURER_CODE,
                                                     GBCS_TUNNELING_FLOW_CONTROL_SUPPORT);
  if (status != EMBER_AF_PLUGIN_TUNNELING_CLIENT_SUCCESS
      && !handleRequestTunnelFailure(status)) {
    return false;
  }

  tunnel.state = RESPONSE_PENDING_TUNNEL;
  return true;
}

// See if we can recover from tunnel creation issues.
static bool handleRequestTunnelFailure(EmberAfPluginTunnelingClientStatus status)
{
  emberAfPluginGbcsGasMeterPrintln("GSME: handleRequestTunnelFailure 0x%x",
                                   status);

  if (status == EMBER_AF_PLUGIN_TUNNELING_CLIENT_BUSY) {
    // Per GBCS Where the GSME receives a RequestTunnelResponse command from the
    // CHF with a TunnelStatus of 0x01 (Busy), the GSME shall send another
    // RequestTunnel command the next time it turns its HAN Interface on.
    // So we'll try again next time sleepyMeterEventCallback() is called.
    tunnel.state = REQUEST_PENDING_TUNNEL;
    return true;
  } else if (status == EMBER_AF_PLUGIN_TUNNELING_CLIENT_NO_MORE_TUNNEL_IDS) {
    // Per GBCS close any other tunnels we may have with the device
    // and once all responses are received try the RequestTunnel again
    // Nothing really we can do because we are only supporting one tunnel
    // to the CHF.
    emberAfAppPrintln("%p%p%p",
                      "Error: ",
                      "Tunnel Create failed: ",
                      "No more tunnel ids");
    tunnel.state = CLOSED_TUNNEL;
    return false;
  }

  // All other errors are either due to mis-configuration or errors that we
  // cannot recover from so print the error and return false.
  emberAfAppPrintln("%p%p%p0x%x",
                    "Error: ",
                    "Tunnel Create failed: ",
                    "Tunneling Client Status: ",
                    status);
  tunnel.state = CLOSED_TUNNEL;
  return false;
}

// Handle GBZ formatted remote party messages
static void handleRemotePartyMessage(uint16_t gbzCommandLen, uint8_t *gbzCommand)
{
  EmberAfGbzMessageParserState gbzParser = { 0 };
  bool validGbzMessage;
  uint16_t headerLen = 0;
  uint16_t gbzResponseLen = 0;
  EmberAfGbzMessageCreatorState gbzCreator = { 0 };
  EmberAfGbzMessageCreatorResult * result;

  // Since this is just a test application we don't actually process the ZCL messages.
  // instead we just send a normal response for each ZCL command in the received
  // GBZ formatted messages.

  // First add the tunneling protocol header
  emberAfCopyInt8u(message, headerLen, TUNNEL_MANAGER_HEADER_PUT);
  headerLen += 1;

  // Add any non GBZ header data from the request
#if GBZ_OFFSET != 0
  MEMCOPY(&(message[headerLen]), gbzCommand, GBZ_OFFSET);
  headerLen += GBZ_OFFSET;
  gbzCommand += GBZ_OFFSET;
  gbzCommandLen -= GBZ_OFFSET;
#endif

  validGbzMessage =   emberAfPluginGbzMessageControllerParserInit(&gbzParser,
                                                                  EMBER_AF_GBZ_MESSAGE_COMMAND,
                                                                  gbzCommand,
                                                                  gbzCommandLen,
                                                                  false,
                                                                  0);
  if (validGbzMessage) {
    // TODO: properly handle the request.  For now we just send a default response to all
    // all requests.

    emberAfPluginGbzMessageControllerCreatorInit(&gbzCreator,
                                                 EMBER_AF_GBZ_MESSAGE_RESPONSE,
                                                 0,
                                                 0,
                                                 0,
                                                 &message[headerLen],
                                                 (TEMP_MESSAGE_LEN - headerLen));

    // Loop through each ZCL command adding a default ZCL response to the GBZ response
    while (emberAfPluginGbzMessageControllerHasNextCommand(&gbzParser)) {
      EmberAfGbzZclCommand zclRequest = { 0 };
      EmberAfGbzZclCommand zclResponse = { 0 };

      uint8_t zclResponsePayload[2];
      emberAfPluginGbzMessageControllerNextCommand(&gbzParser, &zclRequest);

      emberAfPluginGbcsGasMeterPrintln("GSME: received the following ZCL request");
      emberAfPluginGbzMessageControllerPrintCommandInfo(&zclRequest);

      zclResponse.clusterId = zclRequest.clusterId;
      zclResponse.frameControl = 0;
      zclResponse.transactionSequenceNumber = zclRequest.transactionSequenceNumber;
      zclResponse.commandId = ZCL_DEFAULT_RESPONSE_COMMAND_ID;
      zclResponsePayload[0] = zclRequest.commandId;
      zclResponsePayload[1] = EMBER_ZCL_STATUS_SUCCESS;
      zclResponse.payload = &zclResponsePayload[0];
      zclResponse.payloadLength = 2;
      zclResponse.direction = (zclRequest.direction == ZCL_DIRECTION_SERVER_TO_CLIENT)
                              ? ZCL_DIRECTION_CLIENT_TO_SERVER : ZCL_DIRECTION_SERVER_TO_CLIENT;
      zclResponse.frameControl |= ((zclResponse.direction == ZCL_DIRECTION_SERVER_TO_CLIENT)
                                   ? ZCL_FRAME_CONTROL_SERVER_TO_CLIENT : 0);
      zclResponse.clusterSpecific = false;
      zclResponse.mfgSpecific = false;

      emberAfPluginGbcsGasMeterPrintln("GSME: responding with the following ZCL response");
      emberAfPluginGbzMessageControllerPrintCommandInfo(&zclResponse);

      emberAfPluginGbzMessageControllerAppendCommand(&gbzCreator, &zclResponse);
    }

    result = emberAfPluginGbzMessageControllerCreatorAssemble(&gbzCreator);
    gbzResponseLen  = result->payloadLength;
    emberAfPluginGbzMessageControllerCreatorCleanup(&gbzCreator);
    emberAfPluginGbzMessageControllerParserCleanup(&gbzParser);
  } else {
    // This is not a valid GBZ message so for testing purposes we'll just echo
    // the received data back.
    if (gbzCommandLen > 0) {
      MEMCOPY(&(message[headerLen]), gbzCommand, gbzCommandLen);
      gbzResponseLen = gbzCommandLen;
    }
  }

  tunnelSendData(headerLen + gbzResponseLen, message);
}

// Service discovery callback. Passed into
// emberAfFindDevicesByProfileAndCluster below. This callback is not part of
// the application framework callbacks, it is used by the service discovery
// libraries API.
static void serviceDiscoveryCallback(const EmberAfServiceDiscoveryResult* result)
{
  if (state == SERVICE_DISCOVERY_STARTED
      && emberAfHaveDiscoveryResponseStatus(result->status)) {
    uint8_t i;
    uint8_t physAttId[] = {
      LOW_BYTE(ZCL_PHYSICAL_ENVIRONMENT_ATTRIBUTE_ID),
      HIGH_BYTE(ZCL_PHYSICAL_ENVIRONMENT_ATTRIBUTE_ID)
    };
    const EmberAfEndpointList* epList
      = (const EmberAfEndpointList*)result->responseData;

    // Send a physical attribute read to each responding endpoint
    for (i = 0; i < epList->count; i++) {
      emberAfFillCommandGlobalClientToServerReadAttributes(ZCL_BASIC_CLUSTER_ID,
                                                           physAttId,
                                                           sizeof(physAttId));
      emberAfSetCommandEndpoints(GSME_ENDPOINT, epList->list[i]);
      emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, result->matchAddress);
    }
  } else if (result->status == EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE) {
    emberAfAppPrintln("service discovery complete.");
  }
}
