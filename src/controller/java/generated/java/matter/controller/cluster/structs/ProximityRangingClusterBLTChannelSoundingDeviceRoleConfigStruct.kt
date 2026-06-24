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
package matter.controller.cluster.structs

import java.util.Optional
import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ProximityRangingClusterBLTChannelSoundingDeviceRoleConfigStruct(
  val role: UByte,
  val peerBLTDevIK: ByteArray,
  val BLTCSMode: Optional<UByte>,
  val BLTCSSecurityLevel: Optional<UByte>,
  val ltk: Optional<ByteArray>,
) {
  override fun toString(): String = buildString {
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
      if (BLTCSSecurityLevel.isPresent) {
        val optBLTCSSecurityLevel = BLTCSSecurityLevel.get()
        put(ContextSpecificTag(TAG_BLTCS_SECURITY_LEVEL), optBLTCSSecurityLevel)
      }
      if (ltk.isPresent) {
        val optltk = ltk.get()
        put(ContextSpecificTag(TAG_LTK), optltk)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_ROLE = 0
    private const val TAG_PEER_BLT_DEV_IK = 1
    private const val TAG_BLTCS_MODE = 2
    private const val TAG_BLTCS_SECURITY_LEVEL = 3
    private const val TAG_LTK = 4

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): ProximityRangingClusterBLTChannelSoundingDeviceRoleConfigStruct {
      tlvReader.enterStructure(tlvTag)
      val role = tlvReader.getUByte(ContextSpecificTag(TAG_ROLE))
      val peerBLTDevIK = tlvReader.getByteArray(ContextSpecificTag(TAG_PEER_BLT_DEV_IK))
      val BLTCSMode =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_BLTCS_MODE))) {
          Optional.of(tlvReader.getUByte(ContextSpecificTag(TAG_BLTCS_MODE)))
        } else {
          Optional.empty()
        }
      val BLTCSSecurityLevel =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_BLTCS_SECURITY_LEVEL))) {
          Optional.of(tlvReader.getUByte(ContextSpecificTag(TAG_BLTCS_SECURITY_LEVEL)))
        } else {
          Optional.empty()
        }
      val ltk =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_LTK))) {
          Optional.of(tlvReader.getByteArray(ContextSpecificTag(TAG_LTK)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ProximityRangingClusterBLTChannelSoundingDeviceRoleConfigStruct(
        role,
        peerBLTDevIK,
        BLTCSMode,
        BLTCSSecurityLevel,
        ltk,
      )
    }
  }
}
