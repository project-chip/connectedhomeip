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

import matter.controller.MatterController
import matter.devicecontroller.cluster.structs.*

class NetworkCommissioningCluster(
  private val controller: MatterController,
  private val endpointId: UShort
) {
  class ScanNetworksResponse(
    val networkingStatus: UInt,
    val debugText: String?,
    val wiFiScanResults: List<NetworkCommissioningClusterWiFiInterfaceScanResultStruct>?,
    val threadScanResults: List<NetworkCommissioningClusterThreadInterfaceScanResultStruct>?
  )

  class NetworkConfigResponse(
    val networkingStatus: UInt,
    val debugText: String?,
    val networkIndex: UByte?,
    val clientIdentity: ByteArray?,
    val possessionSignature: ByteArray?
  )

  class ConnectNetworkResponse(
    val networkingStatus: UInt,
    val debugText: String?,
    val errorValue: Int?
  )

  class QueryIdentityResponse(val identity: ByteArray, val possessionSignature: ByteArray?)

  class NetworksAttribute(val value: List<NetworkCommissioningClusterNetworkInfoStruct>)

  class LastNetworkingStatusAttribute(val value: UInt?)

  class LastNetworkIDAttribute(val value: ByteArray?)

  class LastConnectErrorValueAttribute(val value: Int?)

  class SupportedWiFiBandsAttribute(val value: List<UInt>?)

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun scanNetworks(
    ssid: ByteArray?,
    breadcrumb: ULong?,
    timedInvokeTimeoutMs: Int? = null
  ): ScanNetworksResponse {
    val commandId = 0L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun addOrUpdateWiFiNetwork(
    ssid: ByteArray,
    credentials: ByteArray,
    breadcrumb: ULong?,
    networkIdentity: ByteArray?,
    clientIdentifier: ByteArray?,
    possessionNonce: ByteArray?,
    timedInvokeTimeoutMs: Int? = null
  ): NetworkConfigResponse {
    val commandId = 2L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun addOrUpdateThreadNetwork(
    operationalDataset: ByteArray,
    breadcrumb: ULong?,
    timedInvokeTimeoutMs: Int? = null
  ): NetworkConfigResponse {
    val commandId = 3L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun removeNetwork(
    networkID: ByteArray,
    breadcrumb: ULong?,
    timedInvokeTimeoutMs: Int? = null
  ): NetworkConfigResponse {
    val commandId = 4L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun connectNetwork(
    networkID: ByteArray,
    breadcrumb: ULong?,
    timedInvokeTimeoutMs: Int? = null
  ): ConnectNetworkResponse {
    val commandId = 6L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun reorderNetwork(
    networkID: ByteArray,
    networkIndex: UByte,
    breadcrumb: ULong?,
    timedInvokeTimeoutMs: Int? = null
  ): NetworkConfigResponse {
    val commandId = 8L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun queryIdentity(
    keyIdentifier: ByteArray,
    possessionNonce: ByteArray?,
    timedInvokeTimeoutMs: Int? = null
  ): QueryIdentityResponse {
    val commandId = 9L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun readMaxNetworksAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeMaxNetworksAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readNetworksAttribute(): NetworksAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeNetworksAttribute(minInterval: Int, maxInterval: Int): NetworksAttribute {
    // Implementation needs to be added here
  }

  suspend fun readScanMaxTimeSecondsAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeScanMaxTimeSecondsAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readConnectMaxTimeSecondsAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeConnectMaxTimeSecondsAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readInterfaceEnabledAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun writeInterfaceEnabledAttribute(value: Boolean, timedWriteTimeoutMs: Int? = null) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
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

  suspend fun readSupportedThreadFeaturesAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeSupportedThreadFeaturesAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readThreadVersionAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeThreadVersionAttribute(minInterval: Int, maxInterval: Int): UShort {
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

  suspend fun readFeatureMapAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun subscribeFeatureMapAttribute(minInterval: Int, maxInterval: Int): UInt {
    // Implementation needs to be added here
  }

  suspend fun readClusterRevisionAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeClusterRevisionAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  companion object {
    const val CLUSTER_ID: UInt = 49u
  }
}
