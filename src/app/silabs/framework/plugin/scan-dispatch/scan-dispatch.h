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
 * @brief Dispatching 802.15.4 scan results to interested parties.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_SCAN_DISPATCH_H
#define SILABS_SCAN_DISPATCH_H

/** @addtogroup scan-dispatch Scan Dispatch
 *
 * This plugin allows for multiple consumers of stack
 * 802.15.4 scan callbacks.
 *
 * @{
 */

/**
 * @brief The size of the dispatch queue.
 */
#ifndef EMBER_AF_PLUGIN_SCAN_DISPATCH_SCAN_QUEUE_SIZE
  #define EMBER_AF_PLUGIN_SCAN_DISPATCH_SCAN_QUEUE_SIZE 10
#endif

/**
 * @brief The information regarding scan results.
 */
typedef struct {
  /** The status indicating the success or failure of a scan.
   *
   * This member is only valid when
   * ::emberAfPluginScanDispatchScanResultsAreComplete returns true.
   */
  EmberStatus status;

  /** The RSSI found during a scan.
   *
   * This member is only valid when
   * ::emberAfPluginScanDispatchScanResultsAreComplete returns false.
   */
  int8_t rssi;

  /** The channel on which the scan is taking place.
   *
   * This member is only valid when the results are for an energy scan or
   * ::emberAfPluginScanDispatchScanResultsAreComplete returns true and
   * ::emberAfPluginScanDispatchScanResultsAreFailure returns false. Note
   * that in active scan results, users can find the channel on which the network
   * was found using the network member of this structure.
   */
  uint8_t channel;

  /** The LQI found during the scan.
   *
   * This member is only valid when the result are for an active scan.
   */
  uint8_t lqi;

  /** The Zigbee network found in the scan.
   *
   * This member is only valid when the result are for an active scan.
   */
  EmberZigbeeNetwork *network;

  /** A mask containing information about the scan. */
  uint16_t mask;
} EmberAfPluginScanDispatchScanResults;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
  #define EM_AF_PLUGIN_SCAN_DISPATCH_SCAN_RESULTS_MASK_SCAN_TYPE (0x00FF)
  #define EM_AF_PLUGIN_SCAN_DISPATCH_SCAN_RESULTS_MASK_COMPLETE  (0x0100)
  #define EM_AF_PLUGIN_SCAN_DISPATCH_SCAN_RESULTS_MASK_FAILURE   (0x0200)
#endif

/** @brief Gets the scan type.
 *
 * Gets the scan type from an ::EmberAfPluginScanDispatchScanResults.
 * This value will either be ::EMBER_ENERGY_SCAN or ::EMBER_ACTIVE_SCAN.
 *
 * @param results The ::EmberAfPluginScanDispatchScanResults for which
 * the scan type will be found.
 *
 * @return The ::EmberNetworkScanType of the scan results.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
EmberNetworkScanType
emberAfPluginScanDispatchScanResultsGetScanType(EmberAfPluginScanDispatchScanResults *results);
#else
  #define emberAfPluginScanDispatchScanResultsGetScanType(results) \
  ((EmberNetworkScanType)((results)->mask                          \
                          & EM_AF_PLUGIN_SCAN_DISPATCH_SCAN_RESULTS_MASK_SCAN_TYPE))
#endif

/** @brief Results are complete.
 *
 * Gets whether or not the scan that was originally requested by the
 * consumer has completed. This can happen either when the dispatcher
 * asks the stack for a scan (see ::emberStartScan) or after all of
 * the scan results have been delivered to the consumer (see
 * ::emberAfScanCompleteCallback).
 *
 * @param results The ::EmberAfPluginScanDispatchScanResults that belong
 * to a potentially complete scan.
 *
 * @return Indicates whether or not the scan for the scan results is complete.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
bool
emberAfPluginScanDispatchScanResultsAreComplete(EmberAfPluginScanDispatchScanResults *results);
#else
  #define emberAfPluginScanDispatchScanResultsAreComplete(results) \
  HIGH_BYTE(((results)->mask                                       \
             & EM_AF_PLUGIN_SCAN_DISPATCH_SCAN_RESULTS_MASK_COMPLETE))
#endif

/** @brief Results in a failure.
 *
 * Gets whether or not the scan results are from a failed call
 * to ::emberStartScan.
 *
 * @param results The ::EmberAfPluginScanDispatchScanResults for which
 * the call to ::emberStartScan may have failed.
 *
 * @return Indicates whether or not the call to ::emberStartScan failed for these
 * results.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
bool
emberAfPluginScanDispatchScanResultsAreFailure(EmberAfPluginScanDispatchScanResults *results);
#else
  #define emberAfPluginScanDispatchScanResultsAreFailure(results) \
  HIGH_BYTE(((results)->mask                                      \
             & EM_AF_PLUGIN_SCAN_DISPATCH_SCAN_RESULTS_MASK_FAILURE))
#endif

/**
 * @brief Handles scan results.
 */
typedef void (*EmberAfPluginScanDispatchScanResultsHandler)(EmberAfPluginScanDispatchScanResults *results);

/**
 * @brief A structure containing data for scheduling a scan.
 */
typedef struct {
  /** The 802.15.4 scan type to be scheduled. */
  EmberNetworkScanType scanType;
  /** The channel mask to be scanned. */
  uint32_t channelMask;
  /** The duration of the scan period, as an exponent. */
  uint8_t duration;
  /** The handler to be called with the scan results. */
  EmberAfPluginScanDispatchScanResultsHandler handler;
} EmberAfPluginScanDispatchScanData;

/** @brief Schedules a scan.
 *
 * This API schedules an 802.15.4 scan. The results will be delivered to
 * the consumer via a handler in the passed ::EmberAfPluginScanDispatchScanData.
 *
 * @param data An ::EmberAfPluginScanDispatchScanData that holds the scanType,
 * channelMask, duration, and ::EmberAfPluginScanDispatchScanResultsHandler for
 * the scan.
 *
 * @return An ::EmberStatus value describing the result of the scheduling of
 * a scan.
 */
EmberStatus emberAfPluginScanDispatchScheduleScan(EmberAfPluginScanDispatchScanData *data);

/** @brief Removes all consumers in the queue.
 *
 * A call to this function will remove all consumers in the queue for scan
 * results. It will also cancel any 802.15.4 scan that the stack is currently
 * performing.
 */
void emberAfPluginScanDispatchClear(void);

// @} END addtogroup

#endif /* __SCAN_DISPATCH_H__ */
