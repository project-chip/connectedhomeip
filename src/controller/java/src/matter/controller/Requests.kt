/*
 *   Copyright (c) 2023 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package matter.controller

/**
 * Information about a invoke request element.
 *
 * @param endpointId invoked command's endpoint Id
 * @param clusterId invoked command's cluster Id
 * @param commandId invoked command's command Id
 * @param tlvByteArray byteArray representation in terms of TLV for invoke command's argument
 */
data class InvokeElement(
  val endpointId: Int,
  val clusterId: Long,
  val commandId: Long,
  val tlvValue: ByteArray
)

/**
 * Information about a subscribe request element.
 *
 * @param minInterval the requested minimum interval boundary floor in seconds
 * @param maxInterval the requested maximum interval boundary ceiling in seconds
 * @param keepSubscriptions If KeepSubscriptions is FALSE, all existing or pending subscriptions on
 *   the publisher for this subscriber SHALL be terminated.
 * @param isFabricFiltered limits the data read within fabric-scoped lists to the accessing fabric
 */
data class SubscribeElement(
  val minInterval: Int,
  val maxInterval: Int,
  val keepSubscriptions: Boolean,
  val isFabricFiltered: Boolean
)

/**
 * A write request representation
 *
 * @param endpoint a UShort endpoint to write into
 * @param cluster a UInt for which cluster on the [endpoint] to write into
 * @param attribute a UInt for which attribute on the cluster to write into
 * @param tlv a tlv byteArray
 * @param dataVersion a optional data version
 */
data class WriteRequest(
  val endpoint: UShort,
  val cluster: UInt,
  val attribute: UInt,
  val tlv: ByteArray,
  val dataVersion: Long? = null
) {
  override fun toString(): String = "$endpoint/$cluster/$attribute/$tlv/$dataVersion"
}
