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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class TargetNavigatorClusterTargetUpdatedEvent(
  val targetList:
    List<chip.devicecontroller.cluster.structs.TargetNavigatorClusterTargetInfoStruct>,
  val currentTarget: UInt,
  val data: ByteArray,
) {
  override fun toString(): String = buildString {
    append("TargetNavigatorClusterTargetUpdatedEvent {\n")
    append("\ttargetList : $targetList\n")
    append("\tcurrentTarget : $currentTarget\n")
    append("\tdata : $data\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      startArray(ContextSpecificTag(TAG_TARGET_LIST))
      for (item in targetList.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
      put(ContextSpecificTag(TAG_CURRENT_TARGET), currentTarget)
      put(ContextSpecificTag(TAG_DATA), data)
      endStructure()
    }
  }

  companion object {
    private const val TAG_TARGET_LIST = 0
    private const val TAG_CURRENT_TARGET = 1
    private const val TAG_DATA = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): TargetNavigatorClusterTargetUpdatedEvent {
      tlvReader.enterStructure(tlvTag)
      val targetList =
        buildList<chip.devicecontroller.cluster.structs.TargetNavigatorClusterTargetInfoStruct> {
          tlvReader.enterArray(ContextSpecificTag(TAG_TARGET_LIST))
          while (!tlvReader.isEndOfContainer()) {
            this.add(
              chip.devicecontroller.cluster.structs.TargetNavigatorClusterTargetInfoStruct.fromTlv(
                AnonymousTag,
                tlvReader,
              )
            )
          }
          tlvReader.exitContainer()
        }
      val currentTarget = tlvReader.getUInt(ContextSpecificTag(TAG_CURRENT_TARGET))
      val data = tlvReader.getByteArray(ContextSpecificTag(TAG_DATA))

      tlvReader.exitContainer()

      return TargetNavigatorClusterTargetUpdatedEvent(targetList, currentTarget, data)
    }
  }
}
