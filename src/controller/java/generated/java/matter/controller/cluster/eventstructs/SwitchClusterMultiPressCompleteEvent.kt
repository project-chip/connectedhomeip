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
package matter.controller.cluster.eventstructs

import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class SwitchClusterMultiPressCompleteEvent(
  val previousPosition: UByte,
  val totalNumberOfPressesCounted: UByte,
) {
  override fun toString(): String = buildString {
    append("SwitchClusterMultiPressCompleteEvent {\n")
    append("\tpreviousPosition : $previousPosition\n")
    append("\ttotalNumberOfPressesCounted : $totalNumberOfPressesCounted\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_PREVIOUS_POSITION), previousPosition)
      put(ContextSpecificTag(TAG_TOTAL_NUMBER_OF_PRESSES_COUNTED), totalNumberOfPressesCounted)
      endStructure()
    }
  }

  companion object {
    private const val TAG_PREVIOUS_POSITION = 0
    private const val TAG_TOTAL_NUMBER_OF_PRESSES_COUNTED = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): SwitchClusterMultiPressCompleteEvent {
      tlvReader.enterStructure(tlvTag)
      val previousPosition = tlvReader.getUByte(ContextSpecificTag(TAG_PREVIOUS_POSITION))
      val totalNumberOfPressesCounted =
        tlvReader.getUByte(ContextSpecificTag(TAG_TOTAL_NUMBER_OF_PRESSES_COUNTED))

      tlvReader.exitContainer()

      return SwitchClusterMultiPressCompleteEvent(previousPosition, totalNumberOfPressesCounted)
    }
  }
}
