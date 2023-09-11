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

class UnitTestingClusterTestListStructOctet(val member1: Long, val member2: ByteArray) {
  override fun toString(): String = buildString {
    append("UnitTestingClusterTestListStructOctet {\n")
    append("\tmember1 : $member1\n")
    append("\tmember2 : $member2\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_MEMBER1), member1)
      put(ContextSpecificTag(TAG_MEMBER2), member2)
      endStructure()
    }
  }

  companion object {
    private const val TAG_MEMBER1 = 0
    private const val TAG_MEMBER2 = 1

    fun fromTlv(tag: Tag, tlvReader: TlvReader): UnitTestingClusterTestListStructOctet {
      tlvReader.enterStructure(tag)
      val member1 = tlvReader.getLong(ContextSpecificTag(TAG_MEMBER1))
      val member2 = tlvReader.getByteArray(ContextSpecificTag(TAG_MEMBER2))

      tlvReader.exitContainer()

      return UnitTestingClusterTestListStructOctet(member1, member2)
    }
  }
}
