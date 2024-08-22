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

class EnergyEvseClusterEnergyTransferStartedEvent(
  val sessionID: UInt,
  val state: UByte,
  val maximumCurrent: Long,
  val maximumDischargeCurrent: Optional<Long>,
) {
  override fun toString(): String = buildString {
    append("EnergyEvseClusterEnergyTransferStartedEvent {\n")
    append("\tsessionID : $sessionID\n")
    append("\tstate : $state\n")
    append("\tmaximumCurrent : $maximumCurrent\n")
    append("\tmaximumDischargeCurrent : $maximumDischargeCurrent\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_SESSION_ID), sessionID)
      put(ContextSpecificTag(TAG_STATE), state)
      put(ContextSpecificTag(TAG_MAXIMUM_CURRENT), maximumCurrent)
      if (maximumDischargeCurrent.isPresent) {
        val optmaximumDischargeCurrent = maximumDischargeCurrent.get()
        put(ContextSpecificTag(TAG_MAXIMUM_DISCHARGE_CURRENT), optmaximumDischargeCurrent)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_SESSION_ID = 0
    private const val TAG_STATE = 1
    private const val TAG_MAXIMUM_CURRENT = 2
    private const val TAG_MAXIMUM_DISCHARGE_CURRENT = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): EnergyEvseClusterEnergyTransferStartedEvent {
      tlvReader.enterStructure(tlvTag)
      val sessionID = tlvReader.getUInt(ContextSpecificTag(TAG_SESSION_ID))
      val state = tlvReader.getUByte(ContextSpecificTag(TAG_STATE))
      val maximumCurrent = tlvReader.getLong(ContextSpecificTag(TAG_MAXIMUM_CURRENT))
      val maximumDischargeCurrent =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MAXIMUM_DISCHARGE_CURRENT))) {
          Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_MAXIMUM_DISCHARGE_CURRENT)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return EnergyEvseClusterEnergyTransferStartedEvent(
        sessionID,
        state,
        maximumCurrent,
        maximumDischargeCurrent,
      )
    }
  }
}
