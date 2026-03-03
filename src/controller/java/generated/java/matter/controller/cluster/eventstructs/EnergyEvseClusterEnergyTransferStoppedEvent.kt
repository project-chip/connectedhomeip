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

class EnergyEvseClusterEnergyTransferStoppedEvent(
  val sessionID: UInt,
  val state: UByte,
  val reason: UByte,
  val energyTransferred: Long,
  val energyDischarged: Optional<Long>,
) {
  override fun toString(): String = buildString {
    append("EnergyEvseClusterEnergyTransferStoppedEvent {\n")
    append("\tsessionID : $sessionID\n")
    append("\tstate : $state\n")
    append("\treason : $reason\n")
    append("\tenergyTransferred : $energyTransferred\n")
    append("\tenergyDischarged : $energyDischarged\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_SESSION_ID), sessionID)
      put(ContextSpecificTag(TAG_STATE), state)
      put(ContextSpecificTag(TAG_REASON), reason)
      put(ContextSpecificTag(TAG_ENERGY_TRANSFERRED), energyTransferred)
      if (energyDischarged.isPresent) {
        val optenergyDischarged = energyDischarged.get()
        put(ContextSpecificTag(TAG_ENERGY_DISCHARGED), optenergyDischarged)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_SESSION_ID = 0
    private const val TAG_STATE = 1
    private const val TAG_REASON = 2
    private const val TAG_ENERGY_TRANSFERRED = 4
    private const val TAG_ENERGY_DISCHARGED = 5

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): EnergyEvseClusterEnergyTransferStoppedEvent {
      tlvReader.enterStructure(tlvTag)
      val sessionID = tlvReader.getUInt(ContextSpecificTag(TAG_SESSION_ID))
      val state = tlvReader.getUByte(ContextSpecificTag(TAG_STATE))
      val reason = tlvReader.getUByte(ContextSpecificTag(TAG_REASON))
      val energyTransferred = tlvReader.getLong(ContextSpecificTag(TAG_ENERGY_TRANSFERRED))
      val energyDischarged =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_ENERGY_DISCHARGED))) {
          Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_ENERGY_DISCHARGED)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return EnergyEvseClusterEnergyTransferStoppedEvent(
        sessionID,
        state,
        reason,
        energyTransferred,
        energyDischarged,
      )
    }
  }
}
