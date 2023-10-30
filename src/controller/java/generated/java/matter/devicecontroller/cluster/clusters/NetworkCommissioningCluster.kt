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
  class ScanNetworksResponse(
    val networkingStatus: UInt,
    val debugText: String?,
    val wiFiScanResults:
      ArrayList<ChipStructs.NetworkCommissioningClusterWiFiInterfaceScanResultStruct>?,
    val threadScanResults:
      ArrayList<ChipStructs.NetworkCommissioningClusterThreadInterfaceScanResultStruct>?
  )

  class NetworkConfigResponse(
    val networkingStatus: UInt,
    val debugText: String?,
    val networkIndex: UByte?
  )

  class ConnectNetworkResponse(
    val networkingStatus: UInt,
    val debugText: String?,
    val errorValue: Int?
  )

  class NetworksAttribute(
    val value: ArrayList<ChipStructs.NetworkCommissioningClusterNetworkInfoStruct>
  )

  class LastNetworkingStatusAttribute(val value: UInt?)

  class LastNetworkIDAttribute(val value: ByteArray?)

  class LastConnectErrorValueAttribute(val value: Int?)

  class SupportedWiFiBandsAttribute(val value: ArrayList<UInt>?)

  class GeneratedCommandListAttribute(val value: ArrayList<UInt>)

  class AcceptedCommandListAttribute(val value: ArrayList<UInt>)

  class EventListAttribute(val value: ArrayList<UInt>)

  class AttributeListAttribute(val value: ArrayList<UInt>)

  suspend fun scanNetworks(ssid: ByteArray?, breadcrumb: ULong?): ScanNetworksResponse {
    // Implementation needs to be added here
  }

  suspend fun scanNetworks(
    ssid: ByteArray?,
    breadcrumb: ULong?,
    timedInvokeTimeoutMs: Int
  ): ScanNetworksResponse {
    // Implementation needs to be added here
  }

  suspend fun addOrUpdateWiFiNetwork(
    ssid: ByteArray,
    credentials: ByteArray,
    breadcrumb: ULong?
  ): NetworkConfigResponse {
    // Implementation needs to be added here
  }

  suspend fun addOrUpdateWiFiNetwork(
    ssid: ByteArray,
    credentials: ByteArray,
    breadcrumb: ULong?,
    timedInvokeTimeoutMs: Int
  ): NetworkConfigResponse {
    // Implementation needs to be added here
  }

  suspend fun addOrUpdateThreadNetwork(
    operationalDataset: ByteArray,
    breadcrumb: ULong?
  ): NetworkConfigResponse {
    // Implementation needs to be added here
  }

  suspend fun addOrUpdateThreadNetwork(
    operationalDataset: ByteArray,
    breadcrumb: ULong?,
    timedInvokeTimeoutMs: Int
  ): NetworkConfigResponse {
    // Implementation needs to be added here
  }

  suspend fun removeNetwork(networkID: ByteArray, breadcrumb: ULong?): NetworkConfigResponse {
    // Implementation needs to be added here
  }

  suspend fun removeNetwork(
    networkID: ByteArray,
    breadcrumb: ULong?,
    timedInvokeTimeoutMs: Int
  ): NetworkConfigResponse {
    // Implementation needs to be added here
  }

  suspend fun connectNetwork(networkID: ByteArray, breadcrumb: ULong?): ConnectNetworkResponse {
    // Implementation needs to be added here
  }

  suspend fun connectNetwork(
    networkID: ByteArray,
    breadcrumb: ULong?,
    timedInvokeTimeoutMs: Int
  ): ConnectNetworkResponse {
    // Implementation needs to be added here
  }

  suspend fun reorderNetwork(
    networkID: ByteArray,
    networkIndex: UByte,
    breadcrumb: ULong?
  ): NetworkConfigResponse {
    // Implementation needs to be added here
  }

  suspend fun reorderNetwork(
    networkID: ByteArray,
    networkIndex: UByte,
    breadcrumb: ULong?,
    timedInvokeTimeoutMs: Int
  ): NetworkConfigResponse {
    // Implementation needs to be added here
  }

  suspend fun readMaxNetworksAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeMaxNetworksAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readNetworksAttribute(): NetworksAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeNetworksAttribute(minInterval: Int, maxInterval: Int): NetworksAttribute {
    // Implementation needs to be added here
  }

  suspend fun readScanMaxTimeSecondsAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeScanMaxTimeSecondsAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readConnectMaxTimeSecondsAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeConnectMaxTimeSecondsAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readInterfaceEnabledAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun writeInterfaceEnabledAttribute(value: Boolean) {
    // Implementation needs to be added here
  }

  suspend fun writeInterfaceEnabledAttribute(value: Boolean, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeInterfaceEnabledAttribute(minInterval: Int, maxInterval: Int): Boolean {
    // Implementation needs to be added here
  }

  suspend fun readLastNetworkingStatusAttribute(): LastNetworkingStatusAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeLastNetworkingStatusAttribute(
    minInterval: Int,
    maxInterval: Int
  ): LastNetworkingStatusAttribute {
    // Implementation needs to be added here
  }

  suspend fun readLastNetworkIDAttribute(): LastNetworkIDAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeLastNetworkIDAttribute(
    minInterval: Int,
    maxInterval: Int
  ): LastNetworkIDAttribute {
    // Implementation needs to be added here
  }

  suspend fun readLastConnectErrorValueAttribute(): LastConnectErrorValueAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeLastConnectErrorValueAttribute(
    minInterval: Int,
    maxInterval: Int
  ): LastConnectErrorValueAttribute {
    // Implementation needs to be added here
  }

  suspend fun readSupportedWiFiBandsAttribute(): SupportedWiFiBandsAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeSupportedWiFiBandsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): SupportedWiFiBandsAttribute {
    // Implementation needs to be added here
  }

  suspend fun readSupportedThreadFeaturesAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeSupportedThreadFeaturesAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readThreadVersionAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeThreadVersionAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readGeneratedCommandListAttribute(): GeneratedCommandListAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeGeneratedCommandListAttribute(
    minInterval: Int,
    maxInterval: Int
  ): GeneratedCommandListAttribute {
    // Implementation needs to be added here
  }

  suspend fun readAcceptedCommandListAttribute(): AcceptedCommandListAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcceptedCommandListAttribute(
    minInterval: Int,
    maxInterval: Int
  ): AcceptedCommandListAttribute {
    // Implementation needs to be added here
  }

  suspend fun readEventListAttribute(): EventListAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeEventListAttribute(minInterval: Int, maxInterval: Int): EventListAttribute {
    // Implementation needs to be added here
  }

  suspend fun readAttributeListAttribute(): AttributeListAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeAttributeListAttribute(
    minInterval: Int,
    maxInterval: Int
  ): AttributeListAttribute {
    // Implementation needs to be added here
  }

  suspend fun readFeatureMapAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeFeatureMapAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readClusterRevisionAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeClusterRevisionAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  companion object {
    const val CLUSTER_ID: UInt = 49u
  }
}
