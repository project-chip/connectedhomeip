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

class OvenModeClusterModeTagStruct(val mfgCode: Optional<UShort>, val value: UShort) {
  override fun toString(): String = buildString {
    append("OvenModeClusterModeTagStruct {\n")
    append("\tmfgCode : $mfgCode\n")
    append("\tvalue : $value\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (mfgCode.isPresent) {
        val optmfgCode = mfgCode.get()
        put(ContextSpecificTag(TAG_MFG_CODE), optmfgCode)
      }
      put(ContextSpecificTag(TAG_VALUE), value)
      endStructure()
    }
  }

  companion object {
    private const val TAG_MFG_CODE = 0
    private const val TAG_VALUE = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): OvenModeClusterModeTagStruct {
      tlvReader.enterStructure(tlvTag)
      val mfgCode =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MFG_CODE))) {
          Optional.of(tlvReader.getUShort(ContextSpecificTag(TAG_MFG_CODE)))
        } else {
          Optional.empty()
        }
      val value = tlvReader.getUShort(ContextSpecificTag(TAG_VALUE))

      tlvReader.exitContainer()

      return OvenModeClusterModeTagStruct(mfgCode, value)
    }
  }
}
