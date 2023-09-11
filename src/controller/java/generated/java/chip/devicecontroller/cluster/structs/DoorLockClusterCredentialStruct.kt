/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package chip.devicecontroller.cluster.structs

import chip.devicecontroller.cluster.*
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

class DoorLockClusterCredentialStruct(val credentialType: Int, val credentialIndex: Int) {
  override fun toString(): String = buildString {
    append("DoorLockClusterCredentialStruct {\n")
    append("\tcredentialType : $credentialType\n")
    append("\tcredentialIndex : $credentialIndex\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_CREDENTIAL_TYPE), credentialType)
      put(ContextSpecificTag(TAG_CREDENTIAL_INDEX), credentialIndex)
      endStructure()
    }
  }

  companion object {
    private const val TAG_CREDENTIAL_TYPE = 0
    private const val TAG_CREDENTIAL_INDEX = 1

    fun fromTlv(tag: Tag, tlvReader: TlvReader): DoorLockClusterCredentialStruct {
      tlvReader.enterStructure(tag)
      val credentialType = tlvReader.getInt(ContextSpecificTag(TAG_CREDENTIAL_TYPE))
      val credentialIndex = tlvReader.getInt(ContextSpecificTag(TAG_CREDENTIAL_INDEX))

      tlvReader.exitContainer()

      return DoorLockClusterCredentialStruct(credentialType, credentialIndex)
    }
  }
}
