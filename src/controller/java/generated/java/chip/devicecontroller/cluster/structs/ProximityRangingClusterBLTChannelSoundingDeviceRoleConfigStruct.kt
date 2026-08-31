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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvParsingException
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

import java.util.Optional

class ProximityRangingClusterBLTChannelSoundingDeviceRoleConfigStruct (
    val role: UInt,
    val peerBLTDevIK: ByteArray,
    val BLTCSMode: Optional<UInt>,
    val BLTCSSecurityLevel: UInt,
    val ltk: ByteArray) {
  override fun toString(): String  = buildString {
    append("ProximityRangingClusterBLTChannelSoundingDeviceRoleConfigStruct {\n")
    append("\trole : $role\n")
    append("\tpeerBLTDevIK : $peerBLTDevIK\n")
    append("\tBLTCSMode : $BLTCSMode\n")
    append("\tBLTCSSecurityLevel : $BLTCSSecurityLevel\n")
    append("\tltk : $ltk\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_ROLE), role)
      put(ContextSpecificTag(TAG_PEER_BLT_DEV_IK), peerBLTDevIK)
      if (BLTCSMode.isPresent) {
      val optBLTCSMode = BLTCSMode.get()
      put(ContextSpecificTag(TAG_BLTCS_MODE), optBLTCSMode)
    }
      put(ContextSpecificTag(TAG_BLTCS_SECURITY_LEVEL), BLTCSSecurityLevel)
      put(ContextSpecificTag(TAG_LTK), ltk)
      endStructure()
    }
  }

  companion object {
    private const val TAG_ROLE = 0
    private const val TAG_PEER_BLT_DEV_IK = 1
    private const val TAG_BLTCS_MODE = 2
    private const val TAG_BLTCS_SECURITY_LEVEL = 3
    private const val TAG_LTK = 4

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : ProximityRangingClusterBLTChannelSoundingDeviceRoleConfigStruct {
      tlvReader.enterStructure(tlvTag)
      val role = tlvReader.getUInt(ContextSpecificTag(TAG_ROLE))
      val peerBLTDevIK = tlvReader.getByteArray(ContextSpecificTag(TAG_PEER_BLT_DEV_IK))
      val BLTCSMode = if (tlvReader.isNextTag(ContextSpecificTag(TAG_BLTCS_MODE))) {
      Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_BLTCS_MODE)))
    } else {
      Optional.empty()
    }
      val BLTCSSecurityLevel = tlvReader.getUInt(ContextSpecificTag(TAG_BLTCS_SECURITY_LEVEL))
      val ltk = tlvReader.getByteArray(ContextSpecificTag(TAG_LTK))
      
      tlvReader.exitContainer()

      return ProximityRangingClusterBLTChannelSoundingDeviceRoleConfigStruct(role, peerBLTDevIK, BLTCSMode, BLTCSSecurityLevel, ltk)
    }
  }
}
