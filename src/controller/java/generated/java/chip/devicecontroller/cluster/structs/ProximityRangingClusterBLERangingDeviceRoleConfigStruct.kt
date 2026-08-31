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

class ProximityRangingClusterBLERangingDeviceRoleConfigStruct (
    val role: UInt,
    val peerBLEDeviceID: ULong,
    val BLERBCSecurityMode: UInt,
    val sessionKey: Optional<ByteArray>) {
  override fun toString(): String  = buildString {
    append("ProximityRangingClusterBLERangingDeviceRoleConfigStruct {\n")
    append("\trole : $role\n")
    append("\tpeerBLEDeviceID : $peerBLEDeviceID\n")
    append("\tBLERBCSecurityMode : $BLERBCSecurityMode\n")
    append("\tsessionKey : $sessionKey\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_ROLE), role)
      put(ContextSpecificTag(TAG_PEER_BLE_DEVICE_ID), peerBLEDeviceID)
      put(ContextSpecificTag(TAG_BLERBC_SECURITY_MODE), BLERBCSecurityMode)
      if (sessionKey.isPresent) {
      val optsessionKey = sessionKey.get()
      put(ContextSpecificTag(TAG_SESSION_KEY), optsessionKey)
    }
      endStructure()
    }
  }

  companion object {
    private const val TAG_ROLE = 0
    private const val TAG_PEER_BLE_DEVICE_ID = 1
    private const val TAG_BLERBC_SECURITY_MODE = 2
    private const val TAG_SESSION_KEY = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : ProximityRangingClusterBLERangingDeviceRoleConfigStruct {
      tlvReader.enterStructure(tlvTag)
      val role = tlvReader.getUInt(ContextSpecificTag(TAG_ROLE))
      val peerBLEDeviceID = tlvReader.getULong(ContextSpecificTag(TAG_PEER_BLE_DEVICE_ID))
      val BLERBCSecurityMode = tlvReader.getUInt(ContextSpecificTag(TAG_BLERBC_SECURITY_MODE))
      val sessionKey = if (tlvReader.isNextTag(ContextSpecificTag(TAG_SESSION_KEY))) {
      Optional.of(tlvReader.getByteArray(ContextSpecificTag(TAG_SESSION_KEY)))
    } else {
      Optional.empty()
    }
      
      tlvReader.exitContainer()

      return ProximityRangingClusterBLERangingDeviceRoleConfigStruct(role, peerBLEDeviceID, BLERBCSecurityMode, sessionKey)
    }
  }
}
