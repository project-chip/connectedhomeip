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

class OperationalStateClusterErrorStateStruct (
    val errorStateID: Int,
    val errorStateLabel: Optional<String>,
    val errorStateDetails: Optional<String>) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("OperationalStateClusterErrorStateStruct {\n")
    builder.append("\terrorStateID : $errorStateID\n")
    builder.append("\terrorStateLabel : $errorStateLabel\n")
    builder.append("\terrorStateDetails : $errorStateDetails\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(0), errorStateID)
    if (errorStateLabel.isPresent) {
      val opterrorStateLabel = errorStateLabel.get()
      tlvWriter.put(ContextSpecificTag(1), opterrorStateLabel)
    }
    if (errorStateDetails.isPresent) {
      val opterrorStateDetails = errorStateDetails.get()
      tlvWriter.put(ContextSpecificTag(2), opterrorStateDetails)
    }
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : OperationalStateClusterErrorStateStruct {
      tlvReader.enterStructure(tag)
      val errorStateID: Int = tlvReader.getInt(ContextSpecificTag(0))
      val errorStateLabel: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(1)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      val errorStateDetails: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(2)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      
      tlvReader.exitContainer()

      return OperationalStateClusterErrorStateStruct(errorStateID, errorStateLabel, errorStateDetails)
    }
  }
}
