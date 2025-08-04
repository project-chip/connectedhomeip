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
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class EnergyEvseClusterFaultEvent(
  val sessionID: ULong?,
  val state: UInt,
  val faultStatePreviousState: UInt,
  val faultStateCurrentState: UInt,
) {
  override fun toString(): String = buildString {
    append("EnergyEvseClusterFaultEvent {\n")
    append("\tsessionID : $sessionID\n")
    append("\tstate : $state\n")
    append("\tfaultStatePreviousState : $faultStatePreviousState\n")
    append("\tfaultStateCurrentState : $faultStateCurrentState\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (sessionID != null) {
        put(ContextSpecificTag(TAG_SESSION_ID), sessionID)
      } else {
        putNull(ContextSpecificTag(TAG_SESSION_ID))
      }
      put(ContextSpecificTag(TAG_STATE), state)
      put(ContextSpecificTag(TAG_FAULT_STATE_PREVIOUS_STATE), faultStatePreviousState)
      put(ContextSpecificTag(TAG_FAULT_STATE_CURRENT_STATE), faultStateCurrentState)
      endStructure()
    }
  }

  companion object {
    private const val TAG_SESSION_ID = 0
    private const val TAG_STATE = 1
    private const val TAG_FAULT_STATE_PREVIOUS_STATE = 2
    private const val TAG_FAULT_STATE_CURRENT_STATE = 4

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): EnergyEvseClusterFaultEvent {
      tlvReader.enterStructure(tlvTag)
      val sessionID =
        if (!tlvReader.isNull()) {
          tlvReader.getULong(ContextSpecificTag(TAG_SESSION_ID))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_SESSION_ID))
          null
        }
      val state = tlvReader.getUInt(ContextSpecificTag(TAG_STATE))
      val faultStatePreviousState =
        tlvReader.getUInt(ContextSpecificTag(TAG_FAULT_STATE_PREVIOUS_STATE))
      val faultStateCurrentState =
        tlvReader.getUInt(ContextSpecificTag(TAG_FAULT_STATE_CURRENT_STATE))

      tlvReader.exitContainer()

      return EnergyEvseClusterFaultEvent(
        sessionID,
        state,
        faultStatePreviousState,
        faultStateCurrentState,
      )
    }
  }
}
