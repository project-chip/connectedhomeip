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
package chip.devicecontroller.cluster.structs

import chip.devicecontroller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class GeneralDiagnosticsClusterDeviceLoadStruct(
  val currentSubscriptions: UInt,
  val currentSubscriptionsForFabric: UInt,
  val totalSubscriptionsEstablished: ULong,
  val totalInteractionModelMessagesSent: ULong,
  val totalInteractionModelMessagesReceived: ULong,
) {
  override fun toString(): String = buildString {
    append("GeneralDiagnosticsClusterDeviceLoadStruct {\n")
    append("\tcurrentSubscriptions : $currentSubscriptions\n")
    append("\tcurrentSubscriptionsForFabric : $currentSubscriptionsForFabric\n")
    append("\ttotalSubscriptionsEstablished : $totalSubscriptionsEstablished\n")
    append("\ttotalInteractionModelMessagesSent : $totalInteractionModelMessagesSent\n")
    append("\ttotalInteractionModelMessagesReceived : $totalInteractionModelMessagesReceived\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_CURRENT_SUBSCRIPTIONS), currentSubscriptions)
      put(ContextSpecificTag(TAG_CURRENT_SUBSCRIPTIONS_FOR_FABRIC), currentSubscriptionsForFabric)
      put(ContextSpecificTag(TAG_TOTAL_SUBSCRIPTIONS_ESTABLISHED), totalSubscriptionsEstablished)
      put(
        ContextSpecificTag(TAG_TOTAL_INTERACTION_MODEL_MESSAGES_SENT),
        totalInteractionModelMessagesSent,
      )
      put(
        ContextSpecificTag(TAG_TOTAL_INTERACTION_MODEL_MESSAGES_RECEIVED),
        totalInteractionModelMessagesReceived,
      )
      endStructure()
    }
  }

  companion object {
    private const val TAG_CURRENT_SUBSCRIPTIONS = 0
    private const val TAG_CURRENT_SUBSCRIPTIONS_FOR_FABRIC = 1
    private const val TAG_TOTAL_SUBSCRIPTIONS_ESTABLISHED = 2
    private const val TAG_TOTAL_INTERACTION_MODEL_MESSAGES_SENT = 3
    private const val TAG_TOTAL_INTERACTION_MODEL_MESSAGES_RECEIVED = 4

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): GeneralDiagnosticsClusterDeviceLoadStruct {
      tlvReader.enterStructure(tlvTag)
      val currentSubscriptions = tlvReader.getUInt(ContextSpecificTag(TAG_CURRENT_SUBSCRIPTIONS))
      val currentSubscriptionsForFabric =
        tlvReader.getUInt(ContextSpecificTag(TAG_CURRENT_SUBSCRIPTIONS_FOR_FABRIC))
      val totalSubscriptionsEstablished =
        tlvReader.getULong(ContextSpecificTag(TAG_TOTAL_SUBSCRIPTIONS_ESTABLISHED))
      val totalInteractionModelMessagesSent =
        tlvReader.getULong(ContextSpecificTag(TAG_TOTAL_INTERACTION_MODEL_MESSAGES_SENT))
      val totalInteractionModelMessagesReceived =
        tlvReader.getULong(ContextSpecificTag(TAG_TOTAL_INTERACTION_MODEL_MESSAGES_RECEIVED))

      tlvReader.exitContainer()

      return GeneralDiagnosticsClusterDeviceLoadStruct(
        currentSubscriptions,
        currentSubscriptionsForFabric,
        totalSubscriptionsEstablished,
        totalInteractionModelMessagesSent,
        totalInteractionModelMessagesReceived,
      )
    }
  }
}
