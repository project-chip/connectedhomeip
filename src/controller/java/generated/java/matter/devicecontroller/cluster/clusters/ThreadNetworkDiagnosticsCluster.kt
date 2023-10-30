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

class ThreadNetworkDiagnosticsCluster(private val endpointId: UShort) {
  class ChannelAttribute(val value: UShort?)

  class RoutingRoleAttribute(val value: UInt?)

  class NetworkNameAttribute(val value: String?)

  class PanIdAttribute(val value: UShort?)

  class ExtendedPanIdAttribute(val value: ULong?)

  class MeshLocalPrefixAttribute(val value: ByteArray?)

  class NeighborTableAttribute(
    val value: ArrayList<ChipStructs.ThreadNetworkDiagnosticsClusterNeighborTableStruct>
  )

  class RouteTableAttribute(
    val value: ArrayList<ChipStructs.ThreadNetworkDiagnosticsClusterRouteTableStruct>
  )

  class PartitionIdAttribute(val value: UInt?)

  class WeightingAttribute(val value: UByte?)

  class DataVersionAttribute(val value: UByte?)

  class StableDataVersionAttribute(val value: UByte?)

  class LeaderRouterIdAttribute(val value: UByte?)

  class ActiveTimestampAttribute(val value: ULong?)

  class PendingTimestampAttribute(val value: ULong?)

  class DelayAttribute(val value: UInt?)

  class SecurityPolicyAttribute(
    val value: ChipStructs.ThreadNetworkDiagnosticsClusterSecurityPolicy?
  )

  class ChannelPage0MaskAttribute(val value: ByteArray?)

  class OperationalDatasetComponentsAttribute(
    val value: ChipStructs.ThreadNetworkDiagnosticsClusterOperationalDatasetComponents?
  )

  class ActiveNetworkFaultsListAttribute(val value: ArrayList<UInt>)

  class GeneratedCommandListAttribute(val value: ArrayList<UInt>)

  class AcceptedCommandListAttribute(val value: ArrayList<UInt>)

  class EventListAttribute(val value: ArrayList<UInt>)

  class AttributeListAttribute(val value: ArrayList<UInt>)

  suspend fun resetCounts() {
    // Implementation needs to be added here
  }

  suspend fun resetCounts(timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun readChannelAttribute(): ChannelAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeChannelAttribute(minInterval: Int, maxInterval: Int): ChannelAttribute {
    // Implementation needs to be added here
  }

  suspend fun readRoutingRoleAttribute(): RoutingRoleAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeRoutingRoleAttribute(
    minInterval: Int,
    maxInterval: Int
  ): RoutingRoleAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNetworkNameAttribute(): NetworkNameAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeNetworkNameAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NetworkNameAttribute {
    // Implementation needs to be added here
  }

  suspend fun readPanIdAttribute(): PanIdAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribePanIdAttribute(minInterval: Int, maxInterval: Int): PanIdAttribute {
    // Implementation needs to be added here
  }

  suspend fun readExtendedPanIdAttribute(): ExtendedPanIdAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeExtendedPanIdAttribute(
    minInterval: Int,
    maxInterval: Int
  ): ExtendedPanIdAttribute {
    // Implementation needs to be added here
  }

  suspend fun readMeshLocalPrefixAttribute(): MeshLocalPrefixAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeMeshLocalPrefixAttribute(
    minInterval: Int,
    maxInterval: Int
  ): MeshLocalPrefixAttribute {
    // Implementation needs to be added here
  }

  suspend fun readOverrunCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeOverrunCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readNeighborTableAttribute(): NeighborTableAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeNeighborTableAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NeighborTableAttribute {
    // Implementation needs to be added here
  }

  suspend fun readRouteTableAttribute(): RouteTableAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeRouteTableAttribute(
    minInterval: Int,
    maxInterval: Int
  ): RouteTableAttribute {
    // Implementation needs to be added here
  }

  suspend fun readPartitionIdAttribute(): PartitionIdAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribePartitionIdAttribute(
    minInterval: Int,
    maxInterval: Int
  ): PartitionIdAttribute {
    // Implementation needs to be added here
  }

  suspend fun readWeightingAttribute(): WeightingAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeWeightingAttribute(minInterval: Int, maxInterval: Int): WeightingAttribute {
    // Implementation needs to be added here
  }

  suspend fun readDataVersionAttribute(): DataVersionAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeDataVersionAttribute(
    minInterval: Int,
    maxInterval: Int
  ): DataVersionAttribute {
    // Implementation needs to be added here
  }

  suspend fun readStableDataVersionAttribute(): StableDataVersionAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeStableDataVersionAttribute(
    minInterval: Int,
    maxInterval: Int
  ): StableDataVersionAttribute {
    // Implementation needs to be added here
  }

  suspend fun readLeaderRouterIdAttribute(): LeaderRouterIdAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeLeaderRouterIdAttribute(
    minInterval: Int,
    maxInterval: Int
  ): LeaderRouterIdAttribute {
    // Implementation needs to be added here
  }

  suspend fun readDetachedRoleCountAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeDetachedRoleCountAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readChildRoleCountAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeChildRoleCountAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRouterRoleCountAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRouterRoleCountAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readLeaderRoleCountAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeLeaderRoleCountAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readAttachAttemptCountAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeAttachAttemptCountAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readPartitionIdChangeCountAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribePartitionIdChangeCountAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readBetterPartitionAttachAttemptCountAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeBetterPartitionAttachAttemptCountAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readParentChangeCountAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeParentChangeCountAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readTxTotalCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeTxTotalCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readTxUnicastCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeTxUnicastCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readTxBroadcastCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeTxBroadcastCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readTxAckRequestedCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeTxAckRequestedCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readTxAckedCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeTxAckedCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readTxNoAckRequestedCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeTxNoAckRequestedCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readTxDataCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeTxDataCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readTxDataPollCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeTxDataPollCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readTxBeaconCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeTxBeaconCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readTxBeaconRequestCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeTxBeaconRequestCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readTxOtherCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeTxOtherCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readTxRetryCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeTxRetryCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readTxDirectMaxRetryExpiryCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeTxDirectMaxRetryExpiryCountAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Long {
    // Implementation needs to be added here
  }

  suspend fun readTxIndirectMaxRetryExpiryCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeTxIndirectMaxRetryExpiryCountAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Long {
    // Implementation needs to be added here
  }

  suspend fun readTxErrCcaCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeTxErrCcaCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readTxErrAbortCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeTxErrAbortCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readTxErrBusyChannelCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeTxErrBusyChannelCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readRxTotalCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeRxTotalCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readRxUnicastCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeRxUnicastCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readRxBroadcastCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeRxBroadcastCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readRxDataCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeRxDataCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readRxDataPollCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeRxDataPollCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readRxBeaconCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeRxBeaconCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readRxBeaconRequestCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeRxBeaconRequestCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readRxOtherCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeRxOtherCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readRxAddressFilteredCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeRxAddressFilteredCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readRxDestAddrFilteredCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeRxDestAddrFilteredCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readRxDuplicatedCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeRxDuplicatedCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readRxErrNoFrameCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeRxErrNoFrameCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readRxErrUnknownNeighborCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeRxErrUnknownNeighborCountAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Long {
    // Implementation needs to be added here
  }

  suspend fun readRxErrInvalidSrcAddrCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeRxErrInvalidSrcAddrCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readRxErrSecCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeRxErrSecCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readRxErrFcsCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeRxErrFcsCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readRxErrOtherCountAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeRxErrOtherCountAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readActiveTimestampAttribute(): ActiveTimestampAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeActiveTimestampAttribute(
    minInterval: Int,
    maxInterval: Int
  ): ActiveTimestampAttribute {
    // Implementation needs to be added here
  }

  suspend fun readPendingTimestampAttribute(): PendingTimestampAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribePendingTimestampAttribute(
    minInterval: Int,
    maxInterval: Int
  ): PendingTimestampAttribute {
    // Implementation needs to be added here
  }

  suspend fun readDelayAttribute(): DelayAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeDelayAttribute(minInterval: Int, maxInterval: Int): DelayAttribute {
    // Implementation needs to be added here
  }

  suspend fun readSecurityPolicyAttribute(): SecurityPolicyAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeSecurityPolicyAttribute(
    minInterval: Int,
    maxInterval: Int
  ): SecurityPolicyAttribute {
    // Implementation needs to be added here
  }

  suspend fun readChannelPage0MaskAttribute(): ChannelPage0MaskAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeChannelPage0MaskAttribute(
    minInterval: Int,
    maxInterval: Int
  ): ChannelPage0MaskAttribute {
    // Implementation needs to be added here
  }

  suspend fun readOperationalDatasetComponentsAttribute(): OperationalDatasetComponentsAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeOperationalDatasetComponentsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): OperationalDatasetComponentsAttribute {
    // Implementation needs to be added here
  }

  suspend fun readActiveNetworkFaultsListAttribute(): ActiveNetworkFaultsListAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeActiveNetworkFaultsListAttribute(
    minInterval: Int,
    maxInterval: Int
  ): ActiveNetworkFaultsListAttribute {
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
    const val CLUSTER_ID: UInt = 53u
  }
}
