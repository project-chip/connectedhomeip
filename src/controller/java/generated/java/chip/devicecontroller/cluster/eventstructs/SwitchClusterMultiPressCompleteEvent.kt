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
package chip.devicecontroller.cluster.eventstructs

import chip.devicecontroller.cluster.*
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvParsingException
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

import java.util.Optional

class SwitchClusterMultiPressCompleteEvent (
    val previousPosition: Int,
    val totalNumberOfPressesCounted: Int) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("SwitchClusterMultiPressCompleteEvent {\n")
    builder.append("\tpreviousPosition : $previousPosition\n")
    builder.append("\ttotalNumberOfPressesCounted : $totalNumberOfPressesCounted\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(TAG_PREVIOUS_POSITION), previousPosition)
    tlvWriter.put(ContextSpecificTag(TAG_TOTAL_NUMBER_OF_PRESSES_COUNTED), totalNumberOfPressesCounted)
    tlvWriter.endStructure()
  }

  companion object {
    private const val TAG_PREVIOUS_POSITION = 0
    private const val TAG_TOTAL_NUMBER_OF_PRESSES_COUNTED = 1

    fun fromTlv(tag: Tag, tlvReader: TlvReader) : SwitchClusterMultiPressCompleteEvent {
      tlvReader.enterStructure(tag)
      val previousPosition: Int = tlvReader.getInt(ContextSpecificTag(TAG_PREVIOUS_POSITION))
      val totalNumberOfPressesCounted: Int = tlvReader.getInt(ContextSpecificTag(TAG_TOTAL_NUMBER_OF_PRESSES_COUNTED))
      
      tlvReader.exitContainer()

      return SwitchClusterMultiPressCompleteEvent(previousPosition, totalNumberOfPressesCounted)
    }
  }
}
