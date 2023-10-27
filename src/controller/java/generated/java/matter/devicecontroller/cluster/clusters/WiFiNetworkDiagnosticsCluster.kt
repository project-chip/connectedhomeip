/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

package matter.devicecontroller.cluster.clusters

import java.util.ArrayList

class WiFiNetworkDiagnosticsCluster(private val endpointId: UShort) {
  companion object {
    const val CLUSTER_ID: UInt = 54u
  }

  fun resetCounts(callback: DefaultClusterCallback) {
    // Implementation needs to be added here
  }

  fun resetCounts(callback: DefaultClusterCallback, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  interface BssidAttributeCallback {
    fun onSuccess(value: ByteArray?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface SecurityTypeAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface WiFiVersionAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface ChannelNumberAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface RssiAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface BeaconLostCountAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface BeaconRxCountAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface PacketMulticastRxCountAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface PacketMulticastTxCountAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface PacketUnicastRxCountAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface PacketUnicastTxCountAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface CurrentMaxRateAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface OverrunCountAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface GeneratedCommandListAttributeCallback {
    fun onSuccess(value: ArrayList<Long>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface AcceptedCommandListAttributeCallback {
    fun onSuccess(value: ArrayList<Long>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface EventListAttributeCallback {
    fun onSuccess(value: ArrayList<Long>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface AttributeListAttributeCallback {
    fun onSuccess(value: ArrayList<Long>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  fun readBssidAttribute(callback: BssidAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeBssidAttribute(
    callback: BssidAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readSecurityTypeAttribute(callback: SecurityTypeAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeSecurityTypeAttribute(
    callback: SecurityTypeAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readWiFiVersionAttribute(callback: WiFiVersionAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeWiFiVersionAttribute(
    callback: WiFiVersionAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readChannelNumberAttribute(callback: ChannelNumberAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeChannelNumberAttribute(
    callback: ChannelNumberAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRssiAttribute(callback: RssiAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeRssiAttribute(callback: RssiAttributeCallback, minInterval: Int, maxInterval: Int) {
    // Implementation needs to be added here
  }

  fun readBeaconLostCountAttribute(callback: BeaconLostCountAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeBeaconLostCountAttribute(
    callback: BeaconLostCountAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readBeaconRxCountAttribute(callback: BeaconRxCountAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeBeaconRxCountAttribute(
    callback: BeaconRxCountAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPacketMulticastRxCountAttribute(callback: PacketMulticastRxCountAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePacketMulticastRxCountAttribute(
    callback: PacketMulticastRxCountAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPacketMulticastTxCountAttribute(callback: PacketMulticastTxCountAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePacketMulticastTxCountAttribute(
    callback: PacketMulticastTxCountAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPacketUnicastRxCountAttribute(callback: PacketUnicastRxCountAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePacketUnicastRxCountAttribute(
    callback: PacketUnicastRxCountAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPacketUnicastTxCountAttribute(callback: PacketUnicastTxCountAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePacketUnicastTxCountAttribute(
    callback: PacketUnicastTxCountAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readCurrentMaxRateAttribute(callback: CurrentMaxRateAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeCurrentMaxRateAttribute(
    callback: CurrentMaxRateAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readOverrunCountAttribute(callback: OverrunCountAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeOverrunCountAttribute(
    callback: OverrunCountAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readGeneratedCommandListAttribute(callback: GeneratedCommandListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeGeneratedCommandListAttribute(
    callback: GeneratedCommandListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readAcceptedCommandListAttribute(callback: AcceptedCommandListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeAcceptedCommandListAttribute(
    callback: AcceptedCommandListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readEventListAttribute(callback: EventListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeEventListAttribute(
    callback: EventListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readAttributeListAttribute(callback: AttributeListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeAttributeListAttribute(
    callback: AttributeListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readFeatureMapAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeFeatureMapAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readClusterRevisionAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeClusterRevisionAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }
}
