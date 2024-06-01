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
import java.util.Optional
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class TargetNavigatorClusterTargetUpdatedEvent(
  val targetList:
    Optional<List<chip.devicecontroller.cluster.structs.TargetNavigatorClusterTargetInfoStruct>>,
  val currentTarget: Optional<UInt>,
  val data: Optional<ByteArray>
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
      if (targetList.isPresent) {
        val opttargetList = targetList.get()
        startArray(ContextSpecificTag(TAG_TARGET_LIST))
        for (item in opttargetList.iterator()) {
          item.toTlv(AnonymousTag, this)
        }
        endArray()
      }
      if (currentTarget.isPresent) {
        val optcurrentTarget = currentTarget.get()
        put(ContextSpecificTag(TAG_CURRENT_TARGET), optcurrentTarget)
      }
      if (data.isPresent) {
        val optdata = data.get()
        put(ContextSpecificTag(TAG_DATA), optdata)
      }
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
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_TARGET_LIST))) {
          Optional.of(
            buildList<
              chip.devicecontroller.cluster.structs.TargetNavigatorClusterTargetInfoStruct
            > {
              tlvReader.enterArray(ContextSpecificTag(TAG_TARGET_LIST))
              while (!tlvReader.isEndOfContainer()) {
                this.add(
                  chip.devicecontroller.cluster.structs.TargetNavigatorClusterTargetInfoStruct
                    .fromTlv(AnonymousTag, tlvReader)
                )
              }
              tlvReader.exitContainer()
            }
          )
        } else {
          Optional.empty()
        }
      val currentTarget =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_CURRENT_TARGET))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_CURRENT_TARGET)))
        } else {
          Optional.empty()
        }
      val data =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_DATA))) {
          Optional.of(tlvReader.getByteArray(ContextSpecificTag(TAG_DATA)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return TargetNavigatorClusterTargetUpdatedEvent(targetList, currentTarget, data)
    }
  }
}
