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

class WiFiNetworkDiagnosticsCluster(
  private val controller: MatterController,
  private val endpointId: UShort
) {
  class BssidAttribute(val value: ByteArray?)

  class SecurityTypeAttribute(val value: UInt?)

  class WiFiVersionAttribute(val value: UInt?)

  class ChannelNumberAttribute(val value: UShort?)

  class RssiAttribute(val value: Byte?)

  class BeaconLostCountAttribute(val value: UInt?)

  class BeaconRxCountAttribute(val value: UInt?)

  class PacketMulticastRxCountAttribute(val value: UInt?)

  class PacketMulticastTxCountAttribute(val value: UInt?)

  class PacketUnicastRxCountAttribute(val value: UInt?)

  class PacketUnicastTxCountAttribute(val value: UInt?)

  class CurrentMaxRateAttribute(val value: ULong?)

  class OverrunCountAttribute(val value: ULong?)

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun resetCounts(timedInvokeTimeoutMs: Int? = null) {
    val commandId = 0L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun readBssidAttribute(): BssidAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeBssidAttribute(minInterval: Int, maxInterval: Int): BssidAttribute {
    // Implementation needs to be added here
  }

  suspend fun readSecurityTypeAttribute(): SecurityTypeAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeSecurityTypeAttribute(
    minInterval: Int,
    maxInterval: Int
  ): SecurityTypeAttribute {
    // Implementation needs to be added here
  }

  suspend fun readWiFiVersionAttribute(): WiFiVersionAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeWiFiVersionAttribute(
    minInterval: Int,
    maxInterval: Int
  ): WiFiVersionAttribute {
    // Implementation needs to be added here
  }

  suspend fun readChannelNumberAttribute(): ChannelNumberAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeChannelNumberAttribute(
    minInterval: Int,
    maxInterval: Int
  ): ChannelNumberAttribute {
    // Implementation needs to be added here
  }

  suspend fun readRssiAttribute(): RssiAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeRssiAttribute(minInterval: Int, maxInterval: Int): RssiAttribute {
    // Implementation needs to be added here
  }

  suspend fun readBeaconLostCountAttribute(): BeaconLostCountAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeBeaconLostCountAttribute(
    minInterval: Int,
    maxInterval: Int
  ): BeaconLostCountAttribute {
    // Implementation needs to be added here
  }

  suspend fun readBeaconRxCountAttribute(): BeaconRxCountAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeBeaconRxCountAttribute(
    minInterval: Int,
    maxInterval: Int
  ): BeaconRxCountAttribute {
    // Implementation needs to be added here
  }

  suspend fun readPacketMulticastRxCountAttribute(): PacketMulticastRxCountAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribePacketMulticastRxCountAttribute(
    minInterval: Int,
    maxInterval: Int
  ): PacketMulticastRxCountAttribute {
    // Implementation needs to be added here
  }

  suspend fun readPacketMulticastTxCountAttribute(): PacketMulticastTxCountAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribePacketMulticastTxCountAttribute(
    minInterval: Int,
    maxInterval: Int
  ): PacketMulticastTxCountAttribute {
    // Implementation needs to be added here
  }

  suspend fun readPacketUnicastRxCountAttribute(): PacketUnicastRxCountAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribePacketUnicastRxCountAttribute(
    minInterval: Int,
    maxInterval: Int
  ): PacketUnicastRxCountAttribute {
    // Implementation needs to be added here
  }

  suspend fun readPacketUnicastTxCountAttribute(): PacketUnicastTxCountAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribePacketUnicastTxCountAttribute(
    minInterval: Int,
    maxInterval: Int
  ): PacketUnicastTxCountAttribute {
    // Implementation needs to be added here
  }

  suspend fun readCurrentMaxRateAttribute(): CurrentMaxRateAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeCurrentMaxRateAttribute(
    minInterval: Int,
    maxInterval: Int
  ): CurrentMaxRateAttribute {
    // Implementation needs to be added here
  }

  suspend fun readOverrunCountAttribute(): OverrunCountAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeOverrunCountAttribute(
    minInterval: Int,
    maxInterval: Int
  ): OverrunCountAttribute {
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
    const val CLUSTER_ID: UInt = 54u
  }
}
