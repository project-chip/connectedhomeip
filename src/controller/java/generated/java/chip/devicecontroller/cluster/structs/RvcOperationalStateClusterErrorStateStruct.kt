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

class RvcOperationalStateClusterErrorStateStruct(
  val errorStateID: UInt,
  val errorStateLabel: Optional<String>,
  val errorStateDetails: Optional<String>,
) {
  override fun toString(): String = buildString {
    append("RvcOperationalStateClusterErrorStateStruct {\n")
    append("\terrorStateID : $errorStateID\n")
    append("\terrorStateLabel : $errorStateLabel\n")
    append("\terrorStateDetails : $errorStateDetails\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_ERROR_STATE_ID), errorStateID)
      if (errorStateLabel.isPresent) {
        val opterrorStateLabel = errorStateLabel.get()
        put(ContextSpecificTag(TAG_ERROR_STATE_LABEL), opterrorStateLabel)
      }
      if (errorStateDetails.isPresent) {
        val opterrorStateDetails = errorStateDetails.get()
        put(ContextSpecificTag(TAG_ERROR_STATE_DETAILS), opterrorStateDetails)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_ERROR_STATE_ID = 0
    private const val TAG_ERROR_STATE_LABEL = 1
    private const val TAG_ERROR_STATE_DETAILS = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): RvcOperationalStateClusterErrorStateStruct {
      tlvReader.enterStructure(tlvTag)
      val errorStateID = tlvReader.getUInt(ContextSpecificTag(TAG_ERROR_STATE_ID))
      val errorStateLabel =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_ERROR_STATE_LABEL))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_ERROR_STATE_LABEL)))
        } else {
          Optional.empty()
        }
      val errorStateDetails =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_ERROR_STATE_DETAILS))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_ERROR_STATE_DETAILS)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return RvcOperationalStateClusterErrorStateStruct(
        errorStateID,
        errorStateLabel,
        errorStateDetails,
      )
    }
  }
}
