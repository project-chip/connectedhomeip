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

class ChannelCluster(private val endpointId: UShort) {
  companion object {
    const val CLUSTER_ID: UInt = 1284u
  }

  fun changeChannel(callback: ChangeChannelResponseCallback, match: String) {
    // Implementation needs to be added here
  }

  fun changeChannel(
    callback: ChangeChannelResponseCallback,
    match: String,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun changeChannelByNumber(
    callback: DefaultClusterCallback,
    majorNumber: Integer,
    minorNumber: Integer
  ) {
    // Implementation needs to be added here
  }

  fun changeChannelByNumber(
    callback: DefaultClusterCallback,
    majorNumber: Integer,
    minorNumber: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun skipChannel(callback: DefaultClusterCallback, count: Integer) {
    // Implementation needs to be added here
  }

  fun skipChannel(callback: DefaultClusterCallback, count: Integer, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  interface ChangeChannelResponseCallback {
    fun onSuccess(status: Integer, data: String?)

    fun onError(error: Exception)
  }

  interface ChannelListAttributeCallback {
    fun onSuccess(value: ArrayList<ChipStructs.ChannelClusterChannelInfoStruct>?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface LineupAttributeCallback {
    fun onSuccess(value: ChipStructs.ChannelClusterLineupInfoStruct?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface CurrentChannelAttributeCallback {
    fun onSuccess(value: ChipStructs.ChannelClusterChannelInfoStruct?)

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

  fun readChannelListAttribute(callback: ChannelListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeChannelListAttribute(
    callback: ChannelListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readLineupAttribute(callback: LineupAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeLineupAttribute(
    callback: LineupAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readCurrentChannelAttribute(callback: CurrentChannelAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeCurrentChannelAttribute(
    callback: CurrentChannelAttributeCallback,
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
