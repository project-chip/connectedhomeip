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

class MessagesClusterMessageResponseOptionStruct(
  val messageResponseID: Optional<ULong>,
  val label: Optional<String>
) {
  override fun toString(): String = buildString {
    append("MessagesClusterMessageResponseOptionStruct {\n")
    append("\tmessageResponseID : $messageResponseID\n")
    append("\tlabel : $label\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (messageResponseID.isPresent) {
        val optmessageResponseID = messageResponseID.get()
        put(ContextSpecificTag(TAG_MESSAGE_RESPONSE_I_D), optmessageResponseID)
      }
      if (label.isPresent) {
        val optlabel = label.get()
        put(ContextSpecificTag(TAG_LABEL), optlabel)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_MESSAGE_RESPONSE_I_D = 0
    private const val TAG_LABEL = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): MessagesClusterMessageResponseOptionStruct {
      tlvReader.enterStructure(tlvTag)
      val messageResponseID =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MESSAGE_RESPONSE_I_D))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_MESSAGE_RESPONSE_I_D)))
        } else {
          Optional.empty()
        }
      val label =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_LABEL))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_LABEL)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return MessagesClusterMessageResponseOptionStruct(messageResponseID, label)
    }
  }
}
