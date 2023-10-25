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
  companion object {
    const val CLUSTER_ID: UInt = 53u
  }

  fun resetCounts(callback: DefaultClusterCallback) {
    // Implementation needs to be added here
  }

  fun resetCounts(callback: DefaultClusterCallback, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  interface ChannelAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface RoutingRoleAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NetworkNameAttributeCallback {
    fun onSuccess(value: String?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface PanIdAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface ExtendedPanIdAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface MeshLocalPrefixAttributeCallback {
    fun onSuccess(value: ByteArray?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NeighborTableAttributeCallback {
    fun onSuccess(value: ArrayList<ChipStructs.ThreadNetworkDiagnosticsClusterNeighborTableStruct>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface RouteTableAttributeCallback {
    fun onSuccess(value: ArrayList<ChipStructs.ThreadNetworkDiagnosticsClusterRouteTableStruct>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface PartitionIdAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface WeightingAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface DataVersionAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface StableDataVersionAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface LeaderRouterIdAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface ActiveTimestampAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface PendingTimestampAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface DelayAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface SecurityPolicyAttributeCallback {
    fun onSuccess(value: ChipStructs.ThreadNetworkDiagnosticsClusterSecurityPolicy?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface ChannelPage0MaskAttributeCallback {
    fun onSuccess(value: ByteArray?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface OperationalDatasetComponentsAttributeCallback {
    fun onSuccess(value: ChipStructs.ThreadNetworkDiagnosticsClusterOperationalDatasetComponents?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface ActiveNetworkFaultsListAttributeCallback {
    fun onSuccess(value: ArrayList<Integer>)

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

  fun readChannelAttribute(callback: ChannelAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeChannelAttribute(
    callback: ChannelAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRoutingRoleAttribute(callback: RoutingRoleAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeRoutingRoleAttribute(
    callback: RoutingRoleAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNetworkNameAttribute(callback: NetworkNameAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeNetworkNameAttribute(
    callback: NetworkNameAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPanIdAttribute(callback: PanIdAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePanIdAttribute(
    callback: PanIdAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readExtendedPanIdAttribute(callback: ExtendedPanIdAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeExtendedPanIdAttribute(
    callback: ExtendedPanIdAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readMeshLocalPrefixAttribute(callback: MeshLocalPrefixAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeMeshLocalPrefixAttribute(
    callback: MeshLocalPrefixAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readOverrunCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeOverrunCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNeighborTableAttribute(callback: NeighborTableAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeNeighborTableAttribute(
    callback: NeighborTableAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRouteTableAttribute(callback: RouteTableAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeRouteTableAttribute(
    callback: RouteTableAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPartitionIdAttribute(callback: PartitionIdAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePartitionIdAttribute(
    callback: PartitionIdAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readWeightingAttribute(callback: WeightingAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeWeightingAttribute(
    callback: WeightingAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readDataVersionAttribute(callback: DataVersionAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeDataVersionAttribute(
    callback: DataVersionAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readStableDataVersionAttribute(callback: StableDataVersionAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeStableDataVersionAttribute(
    callback: StableDataVersionAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readLeaderRouterIdAttribute(callback: LeaderRouterIdAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeLeaderRouterIdAttribute(
    callback: LeaderRouterIdAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readDetachedRoleCountAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeDetachedRoleCountAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readChildRoleCountAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeChildRoleCountAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRouterRoleCountAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeRouterRoleCountAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readLeaderRoleCountAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeLeaderRoleCountAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readAttachAttemptCountAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeAttachAttemptCountAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPartitionIdChangeCountAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePartitionIdChangeCountAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readBetterPartitionAttachAttemptCountAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeBetterPartitionAttachAttemptCountAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readParentChangeCountAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeParentChangeCountAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readTxTotalCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeTxTotalCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readTxUnicastCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeTxUnicastCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readTxBroadcastCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeTxBroadcastCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readTxAckRequestedCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeTxAckRequestedCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readTxAckedCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeTxAckedCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readTxNoAckRequestedCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeTxNoAckRequestedCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readTxDataCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeTxDataCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readTxDataPollCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeTxDataPollCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readTxBeaconCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeTxBeaconCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readTxBeaconRequestCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeTxBeaconRequestCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readTxOtherCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeTxOtherCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readTxRetryCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeTxRetryCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readTxDirectMaxRetryExpiryCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeTxDirectMaxRetryExpiryCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readTxIndirectMaxRetryExpiryCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeTxIndirectMaxRetryExpiryCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readTxErrCcaCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeTxErrCcaCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readTxErrAbortCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeTxErrAbortCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readTxErrBusyChannelCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeTxErrBusyChannelCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRxTotalCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeRxTotalCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRxUnicastCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeRxUnicastCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRxBroadcastCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeRxBroadcastCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRxDataCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeRxDataCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRxDataPollCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeRxDataPollCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRxBeaconCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeRxBeaconCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRxBeaconRequestCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeRxBeaconRequestCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRxOtherCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeRxOtherCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRxAddressFilteredCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeRxAddressFilteredCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRxDestAddrFilteredCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeRxDestAddrFilteredCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRxDuplicatedCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeRxDuplicatedCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRxErrNoFrameCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeRxErrNoFrameCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRxErrUnknownNeighborCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeRxErrUnknownNeighborCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRxErrInvalidSrcAddrCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeRxErrInvalidSrcAddrCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRxErrSecCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeRxErrSecCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRxErrFcsCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeRxErrFcsCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRxErrOtherCountAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeRxErrOtherCountAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readActiveTimestampAttribute(callback: ActiveTimestampAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeActiveTimestampAttribute(
    callback: ActiveTimestampAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPendingTimestampAttribute(callback: PendingTimestampAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePendingTimestampAttribute(
    callback: PendingTimestampAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readDelayAttribute(callback: DelayAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeDelayAttribute(
    callback: DelayAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readSecurityPolicyAttribute(callback: SecurityPolicyAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeSecurityPolicyAttribute(
    callback: SecurityPolicyAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readChannelPage0MaskAttribute(callback: ChannelPage0MaskAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeChannelPage0MaskAttribute(
    callback: ChannelPage0MaskAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readOperationalDatasetComponentsAttribute(
    callback: OperationalDatasetComponentsAttributeCallback
  ) {
    // Implementation needs to be added here
  }

  fun subscribeOperationalDatasetComponentsAttribute(
    callback: OperationalDatasetComponentsAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readActiveNetworkFaultsListAttribute(callback: ActiveNetworkFaultsListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeActiveNetworkFaultsListAttribute(
    callback: ActiveNetworkFaultsListAttributeCallback,
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
