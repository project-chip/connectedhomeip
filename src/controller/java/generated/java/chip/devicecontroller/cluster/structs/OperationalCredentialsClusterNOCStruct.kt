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

class OperationalCredentialsClusterNOCStruct(
  val noc: ByteArray,
  val icac: ByteArray?,
  val vvsc: Optional<ByteArray>,
  val fabricIndex: UInt,
) {
  override fun toString(): String = buildString {
    append("OperationalCredentialsClusterNOCStruct {\n")
    append("\tnoc : $noc\n")
    append("\ticac : $icac\n")
    append("\tvvsc : $vvsc\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_NOC), noc)
      if (icac != null) {
        put(ContextSpecificTag(TAG_ICAC), icac)
      } else {
        putNull(ContextSpecificTag(TAG_ICAC))
      }
      if (vvsc.isPresent) {
        val optvvsc = vvsc.get()
        put(ContextSpecificTag(TAG_VVSC), optvvsc)
      }
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
      endStructure()
    }
  }

  companion object {
    private const val TAG_NOC = 1
    private const val TAG_ICAC = 2
    private const val TAG_VVSC = 3
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): OperationalCredentialsClusterNOCStruct {
      tlvReader.enterStructure(tlvTag)
      val noc = tlvReader.getByteArray(ContextSpecificTag(TAG_NOC))
      val icac =
        if (!tlvReader.isNull()) {
          tlvReader.getByteArray(ContextSpecificTag(TAG_ICAC))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_ICAC))
          null
        }
      val vvsc =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_VVSC))) {
          Optional.of(tlvReader.getByteArray(ContextSpecificTag(TAG_VVSC)))
        } else {
          Optional.empty()
        }
      val fabricIndex = tlvReader.getUInt(ContextSpecificTag(TAG_FABRIC_INDEX))

      tlvReader.exitContainer()

      return OperationalCredentialsClusterNOCStruct(noc, icac, vvsc, fabricIndex)
    }
  }
}
