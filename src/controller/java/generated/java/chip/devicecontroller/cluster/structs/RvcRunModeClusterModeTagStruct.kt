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
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvParsingException
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

import java.util.Optional

class RvcRunModeClusterModeTagStruct (
    val mfgCode: Optional<Int>,
    val value: Int) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("RvcRunModeClusterModeTagStruct {\n")
    builder.append("\tmfgCode : $mfgCode\n")
    builder.append("\tvalue : $value\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    if (mfgCode.isPresent) {
      val optmfgCode = mfgCode.get()
      tlvWriter.put(ContextSpecificTag(TAG_MFG_CODE), optmfgCode)
    }
    tlvWriter.put(ContextSpecificTag(TAG_VALUE), value)
    tlvWriter.endStructure()
  }

  companion object {
    private const val TAG_MFG_CODE = 0
    private const val TAG_VALUE = 1

    fun fromTlv(tag: Tag, tlvReader: TlvReader) : RvcRunModeClusterModeTagStruct {
      tlvReader.enterStructure(tag)
      val mfgCode: Optional<Int> = try {
      Optional.of(tlvReader.getInt(ContextSpecificTag(TAG_MFG_CODE)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      val value: Int = tlvReader.getInt(ContextSpecificTag(TAG_VALUE))
      
      tlvReader.exitContainer()

      return RvcRunModeClusterModeTagStruct(mfgCode, value)
    }
  }
}
