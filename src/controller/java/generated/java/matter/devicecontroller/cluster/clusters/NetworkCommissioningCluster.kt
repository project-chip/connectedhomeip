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

class NetworkCommissioningCluster(private val endpointId: UShort) {
  companion object {
    const val CLUSTER_ID: UInt = 49u
  }

  fun scanNetworks(callback: ScanNetworksResponseCallback, ssid: ByteArray?, breadcrumb: Long?) {
    // Implementation needs to be added here
  }

  fun scanNetworks(
    callback: ScanNetworksResponseCallback,
    ssid: ByteArray?,
    breadcrumb: Long?,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun addOrUpdateWiFiNetwork(
    callback: NetworkConfigResponseCallback,
    ssid: ByteArray,
    credentials: ByteArray,
    breadcrumb: Long?
  ) {
    // Implementation needs to be added here
  }

  fun addOrUpdateWiFiNetwork(
    callback: NetworkConfigResponseCallback,
    ssid: ByteArray,
    credentials: ByteArray,
    breadcrumb: Long?,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun addOrUpdateThreadNetwork(
    callback: NetworkConfigResponseCallback,
    operationalDataset: ByteArray,
    breadcrumb: Long?
  ) {
    // Implementation needs to be added here
  }

  fun addOrUpdateThreadNetwork(
    callback: NetworkConfigResponseCallback,
    operationalDataset: ByteArray,
    breadcrumb: Long?,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun removeNetwork(
    callback: NetworkConfigResponseCallback,
    networkID: ByteArray,
    breadcrumb: Long?
  ) {
    // Implementation needs to be added here
  }

  fun removeNetwork(
    callback: NetworkConfigResponseCallback,
    networkID: ByteArray,
    breadcrumb: Long?,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun connectNetwork(
    callback: ConnectNetworkResponseCallback,
    networkID: ByteArray,
    breadcrumb: Long?
  ) {
    // Implementation needs to be added here
  }

  fun connectNetwork(
    callback: ConnectNetworkResponseCallback,
    networkID: ByteArray,
    breadcrumb: Long?,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun reorderNetwork(
    callback: NetworkConfigResponseCallback,
    networkID: ByteArray,
    networkIndex: Integer,
    breadcrumb: Long?
  ) {
    // Implementation needs to be added here
  }

  fun reorderNetwork(
    callback: NetworkConfigResponseCallback,
    networkID: ByteArray,
    networkIndex: Integer,
    breadcrumb: Long?,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  interface ScanNetworksResponseCallback {
    fun onSuccess(
      networkingStatus: Integer,
      debugText: String?,
      wiFiScanResults:
        ArrayList<ChipStructs.NetworkCommissioningClusterWiFiInterfaceScanResultStruct>?,
      threadScanResults:
        ArrayList<ChipStructs.NetworkCommissioningClusterThreadInterfaceScanResultStruct>?
    )

    fun onError(error: Exception)
  }

  interface NetworkConfigResponseCallback {
    fun onSuccess(networkingStatus: Integer, debugText: String?, networkIndex: Integer?)

    fun onError(error: Exception)
  }

  interface ConnectNetworkResponseCallback {
    fun onSuccess(networkingStatus: Integer, debugText: String?, errorValue: Long?)

    fun onError(error: Exception)
  }

  interface NetworksAttributeCallback {
    fun onSuccess(value: ArrayList<ChipStructs.NetworkCommissioningClusterNetworkInfoStruct>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface LastNetworkingStatusAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface LastNetworkIDAttributeCallback {
    fun onSuccess(value: ByteArray?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface LastConnectErrorValueAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface SupportedWiFiBandsAttributeCallback {
    fun onSuccess(value: ArrayList<Integer>?)

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

  fun readMaxNetworksAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeMaxNetworksAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNetworksAttribute(callback: NetworksAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeNetworksAttribute(
    callback: NetworksAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readScanMaxTimeSecondsAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeScanMaxTimeSecondsAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readConnectMaxTimeSecondsAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeConnectMaxTimeSecondsAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readInterfaceEnabledAttribute(callback: BooleanAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeInterfaceEnabledAttribute(callback: DefaultClusterCallback, value: Boolean) {
    // Implementation needs to be added here
  }

  fun writeInterfaceEnabledAttribute(
    callback: DefaultClusterCallback,
    value: Boolean,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeInterfaceEnabledAttribute(
    callback: BooleanAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readLastNetworkingStatusAttribute(callback: LastNetworkingStatusAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeLastNetworkingStatusAttribute(
    callback: LastNetworkingStatusAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readLastNetworkIDAttribute(callback: LastNetworkIDAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeLastNetworkIDAttribute(
    callback: LastNetworkIDAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readLastConnectErrorValueAttribute(callback: LastConnectErrorValueAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeLastConnectErrorValueAttribute(
    callback: LastConnectErrorValueAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readSupportedWiFiBandsAttribute(callback: SupportedWiFiBandsAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeSupportedWiFiBandsAttribute(
    callback: SupportedWiFiBandsAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readSupportedThreadFeaturesAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeSupportedThreadFeaturesAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readThreadVersionAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeThreadVersionAttribute(
    callback: IntegerAttributeCallback,
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
