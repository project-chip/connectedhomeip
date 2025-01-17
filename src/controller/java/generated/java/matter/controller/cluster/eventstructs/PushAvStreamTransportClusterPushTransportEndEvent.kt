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
package matter.controller.cluster.eventstructs

import java.util.Optional
import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class PushAvStreamTransportClusterPushTransportEndEvent(
  val connectionID: UShort,
  val triggerType: UByte,
  val activationReason: Optional<UByte>,
) {
  override fun toString(): String = buildString {
    append("PushAvStreamTransportClusterPushTransportEndEvent {\n")
    append("\tconnectionID : $connectionID\n")
    append("\ttriggerType : $triggerType\n")
    append("\tactivationReason : $activationReason\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_CONNECTION_ID), connectionID)
      put(ContextSpecificTag(TAG_TRIGGER_TYPE), triggerType)
      if (activationReason.isPresent) {
        val optactivationReason = activationReason.get()
        put(ContextSpecificTag(TAG_ACTIVATION_REASON), optactivationReason)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_CONNECTION_ID = 0
    private const val TAG_TRIGGER_TYPE = 1
    private const val TAG_ACTIVATION_REASON = 2

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): PushAvStreamTransportClusterPushTransportEndEvent {
      tlvReader.enterStructure(tlvTag)
      val connectionID = tlvReader.getUShort(ContextSpecificTag(TAG_CONNECTION_ID))
      val triggerType = tlvReader.getUByte(ContextSpecificTag(TAG_TRIGGER_TYPE))
      val activationReason =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_ACTIVATION_REASON))) {
          Optional.of(tlvReader.getUByte(ContextSpecificTag(TAG_ACTIVATION_REASON)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return PushAvStreamTransportClusterPushTransportEndEvent(
        connectionID,
        triggerType,
        activationReason,
      )
    }
  }
}
