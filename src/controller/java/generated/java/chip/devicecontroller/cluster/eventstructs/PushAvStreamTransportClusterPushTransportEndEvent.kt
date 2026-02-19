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
package chip.devicecontroller.cluster.eventstructs

import chip.devicecontroller.cluster.*
import java.util.Optional
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class PushAvStreamTransportClusterPushTransportEndEvent(
  val connectionID: UInt,
  val containerType: UInt,
  val CMAFSessionNumber: Optional<ULong>,
) {
  override fun toString(): String = buildString {
    append("PushAvStreamTransportClusterPushTransportEndEvent {\n")
    append("\tconnectionID : $connectionID\n")
    append("\tcontainerType : $containerType\n")
    append("\tCMAFSessionNumber : $CMAFSessionNumber\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_CONNECTION_ID), connectionID)
      put(ContextSpecificTag(TAG_CONTAINER_TYPE), containerType)
      if (CMAFSessionNumber.isPresent) {
        val optCMAFSessionNumber = CMAFSessionNumber.get()
        put(ContextSpecificTag(TAG_CMAF_SESSION_NUMBER), optCMAFSessionNumber)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_CONNECTION_ID = 0
    private const val TAG_CONTAINER_TYPE = 1
    private const val TAG_CMAF_SESSION_NUMBER = 2

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): PushAvStreamTransportClusterPushTransportEndEvent {
      tlvReader.enterStructure(tlvTag)
      val connectionID = tlvReader.getUInt(ContextSpecificTag(TAG_CONNECTION_ID))
      val containerType = tlvReader.getUInt(ContextSpecificTag(TAG_CONTAINER_TYPE))
      val CMAFSessionNumber =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_CMAF_SESSION_NUMBER))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_CMAF_SESSION_NUMBER)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return PushAvStreamTransportClusterPushTransportEndEvent(
        connectionID,
        containerType,
        CMAFSessionNumber,
      )
    }
  }
}
