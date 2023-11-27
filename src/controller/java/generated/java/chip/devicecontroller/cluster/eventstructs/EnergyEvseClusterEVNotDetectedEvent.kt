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

class EnergyEvseClusterEVNotDetectedEvent(
  val sessionID: ULong,
  val state: UInt,
  val sessionDuration: ULong,
  val sessionEnergyCharged: Long,
  val sessionEnergyDischarged: Optional<Long>
) {
  override fun toString(): String = buildString {
    append("EnergyEvseClusterEVNotDetectedEvent {\n")
    append("\tsessionID : $sessionID\n")
    append("\tstate : $state\n")
    append("\tsessionDuration : $sessionDuration\n")
    append("\tsessionEnergyCharged : $sessionEnergyCharged\n")
    append("\tsessionEnergyDischarged : $sessionEnergyDischarged\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_SESSION_I_D), sessionID)
      put(ContextSpecificTag(TAG_STATE), state)
      put(ContextSpecificTag(TAG_SESSION_DURATION), sessionDuration)
      put(ContextSpecificTag(TAG_SESSION_ENERGY_CHARGED), sessionEnergyCharged)
      if (sessionEnergyDischarged.isPresent) {
        val optsessionEnergyDischarged = sessionEnergyDischarged.get()
        put(ContextSpecificTag(TAG_SESSION_ENERGY_DISCHARGED), optsessionEnergyDischarged)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_SESSION_I_D = 0
    private const val TAG_STATE = 1
    private const val TAG_SESSION_DURATION = 2
    private const val TAG_SESSION_ENERGY_CHARGED = 3
    private const val TAG_SESSION_ENERGY_DISCHARGED = 4

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): EnergyEvseClusterEVNotDetectedEvent {
      tlvReader.enterStructure(tlvTag)
      val sessionID = tlvReader.getULong(ContextSpecificTag(TAG_SESSION_I_D))
      val state = tlvReader.getUInt(ContextSpecificTag(TAG_STATE))
      val sessionDuration = tlvReader.getULong(ContextSpecificTag(TAG_SESSION_DURATION))
      val sessionEnergyCharged = tlvReader.getLong(ContextSpecificTag(TAG_SESSION_ENERGY_CHARGED))
      val sessionEnergyDischarged =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_SESSION_ENERGY_DISCHARGED))) {
          Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_SESSION_ENERGY_DISCHARGED)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return EnergyEvseClusterEVNotDetectedEvent(
        sessionID,
        state,
        sessionDuration,
        sessionEnergyCharged,
        sessionEnergyDischarged
      )
    }
  }
}
