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
import java.util.Optional
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ProximityRangingClusterWiFiRangingDeviceRoleConfigStruct(
  val role: UInt,
  val peerWiFiDevIK: ByteArray,
  val pmk: Optional<ByteArray>,
) {
  override fun toString(): String = buildString {
    append("ProximityRangingClusterWiFiRangingDeviceRoleConfigStruct {\n")
    append("\trole : $role\n")
    append("\tpeerWiFiDevIK : $peerWiFiDevIK\n")
    append("\tpmk : $pmk\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_ROLE), role)
      put(ContextSpecificTag(TAG_PEER_WI_FI_DEV_IK), peerWiFiDevIK)
      if (pmk.isPresent) {
        val optpmk = pmk.get()
        put(ContextSpecificTag(TAG_PMK), optpmk)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_ROLE = 0
    private const val TAG_PEER_WI_FI_DEV_IK = 1
    private const val TAG_PMK = 2

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): ProximityRangingClusterWiFiRangingDeviceRoleConfigStruct {
      tlvReader.enterStructure(tlvTag)
      val role = tlvReader.getUInt(ContextSpecificTag(TAG_ROLE))
      val peerWiFiDevIK = tlvReader.getByteArray(ContextSpecificTag(TAG_PEER_WI_FI_DEV_IK))
      val pmk =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_PMK))) {
          Optional.of(tlvReader.getByteArray(ContextSpecificTag(TAG_PMK)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ProximityRangingClusterWiFiRangingDeviceRoleConfigStruct(role, peerWiFiDevIK, pmk)
    }
  }
}
