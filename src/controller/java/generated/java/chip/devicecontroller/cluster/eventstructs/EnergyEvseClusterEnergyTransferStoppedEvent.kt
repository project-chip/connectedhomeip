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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvParsingException
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

import java.util.Optional

class EnergyEvseClusterEnergyTransferStoppedEvent (
    val sessionID: ULong,
    val state: UInt,
    val reason: UInt,
    val energyTransferred: Long) {
  override fun toString(): String  = buildString {
    append("EnergyEvseClusterEnergyTransferStoppedEvent {\n")
    append("\tsessionID : $sessionID\n")
    append("\tstate : $state\n")
    append("\treason : $reason\n")
    append("\tenergyTransferred : $energyTransferred\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_SESSION_I_D), sessionID)
      put(ContextSpecificTag(TAG_STATE), state)
      put(ContextSpecificTag(TAG_REASON), reason)
      put(ContextSpecificTag(TAG_ENERGY_TRANSFERRED), energyTransferred)
      endStructure()
    }
  }

  companion object {
    private const val TAG_SESSION_I_D = 0
    private const val TAG_STATE = 1
    private const val TAG_REASON = 2
    private const val TAG_ENERGY_TRANSFERRED = 4

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : EnergyEvseClusterEnergyTransferStoppedEvent {
      tlvReader.enterStructure(tlvTag)
      val sessionID = tlvReader.getULong(ContextSpecificTag(TAG_SESSION_I_D))
      val state = tlvReader.getUInt(ContextSpecificTag(TAG_STATE))
      val reason = tlvReader.getUInt(ContextSpecificTag(TAG_REASON))
      val energyTransferred = tlvReader.getLong(ContextSpecificTag(TAG_ENERGY_TRANSFERRED))
      
      tlvReader.exitContainer()

      return EnergyEvseClusterEnergyTransferStoppedEvent(sessionID, state, reason, energyTransferred)
    }
  }
}
