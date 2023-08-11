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
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter
import java.util.Optional

class DescriptorClusterSemanticTagStruct(
  val mfgCode: Int?,
  val namespaceID: Int,
  val tag: Int,
  val label: Optional<String>?
) {
  override fun toString(): String = buildString {
    append("DescriptorClusterSemanticTagStruct {\n")
    append("\tmfgCode : $mfgCode\n")
    append("\tnamespaceID : $namespaceID\n")
    append("\ttag : $tag\n")
    append("\tlabel : $label\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      if (mfgCode != null) {
        put(ContextSpecificTag(TAG_MFG_CODE), mfgCode)
      } else {
        putNull(ContextSpecificTag(TAG_MFG_CODE))
      }
      put(ContextSpecificTag(TAG_NAMESPACE_I_D), namespaceID)
      put(ContextSpecificTag(TAG_TAG), tag)
      if (label != null) {
        if (label.isPresent) {
          val optlabel = label.get()
          put(ContextSpecificTag(TAG_LABEL), optlabel)
        }
      } else {
        putNull(ContextSpecificTag(TAG_LABEL))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_MFG_CODE = 0
    private const val TAG_NAMESPACE_I_D = 1
    private const val TAG_TAG = 2
    private const val TAG_LABEL = 3

    fun fromTlv(tag: Tag, tlvReader: TlvReader): DescriptorClusterSemanticTagStruct {
      tlvReader.enterStructure(tag)
      val mfgCode =
        if (!tlvReader.isNull()) {
          tlvReader.getInt(ContextSpecificTag(TAG_MFG_CODE))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_MFG_CODE))
          null
        }
      val namespaceID = tlvReader.getInt(ContextSpecificTag(TAG_NAMESPACE_I_D))
      val tag = tlvReader.getInt(ContextSpecificTag(TAG_TAG))
      val label =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_LABEL))) {
            Optional.of(tlvReader.getString(ContextSpecificTag(TAG_LABEL)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_LABEL))
          null
        }

      tlvReader.exitContainer()

      return DescriptorClusterSemanticTagStruct(mfgCode, namespaceID, tag, label)
    }
  }
}
