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

class UnitTestingClusterTestListStructOctet (
    val member1: ULong,
    val member2: ByteArray) {
  override fun toString(): String  = buildString {
    append("UnitTestingClusterTestListStructOctet {\n")
    append("\tmember1 : $member1\n")
    append("\tmember2 : $member2\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_MEMBER1), member1)
      put(ContextSpecificTag(TAG_MEMBER2), member2)
      endStructure()
    }
  }

  companion object {
    private const val TAG_MEMBER1 = 0
    private const val TAG_MEMBER2 = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : UnitTestingClusterTestListStructOctet {
      tlvReader.enterStructure(tlvTag)
      val member1 = tlvReader.getULong(ContextSpecificTag(TAG_MEMBER1))
      val member2 = tlvReader.getByteArray(ContextSpecificTag(TAG_MEMBER2))
      
      tlvReader.exitContainer()

      return UnitTestingClusterTestListStructOctet(member1, member2)
    }
  }
}
