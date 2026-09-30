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

class AmbientSensingUnionClusterUnionContributorStatusChangedEvent(
  val contributorStatusChange:
    List<
      chip.devicecontroller.cluster.structs.AmbientSensingUnionClusterContributorStatusChangeStruct
    >,
  val fabricIndex: UInt,
) {
  override fun toString(): String = buildString {
    append("AmbientSensingUnionClusterUnionContributorStatusChangedEvent {\n")
    append("\tcontributorStatusChange : $contributorStatusChange\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      startArray(ContextSpecificTag(TAG_CONTRIBUTOR_STATUS_CHANGE))
      for (item in contributorStatusChange.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
      endStructure()
    }
  }

  companion object {
    private const val TAG_CONTRIBUTOR_STATUS_CHANGE = 0
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): AmbientSensingUnionClusterUnionContributorStatusChangedEvent {
      tlvReader.enterStructure(tlvTag)
      val contributorStatusChange =
        buildList<
          chip.devicecontroller.cluster.structs.AmbientSensingUnionClusterContributorStatusChangeStruct
        > {
          tlvReader.enterArray(ContextSpecificTag(TAG_CONTRIBUTOR_STATUS_CHANGE))
          while (!tlvReader.isEndOfContainer()) {
            this.add(
              chip.devicecontroller.cluster.structs
                .AmbientSensingUnionClusterContributorStatusChangeStruct
                .fromTlv(AnonymousTag, tlvReader)
            )
          }
          tlvReader.exitContainer()
        }
      val fabricIndex = tlvReader.getUInt(ContextSpecificTag(TAG_FABRIC_INDEX))

      tlvReader.exitContainer()

      return AmbientSensingUnionClusterUnionContributorStatusChangedEvent(
        contributorStatusChange,
        fabricIndex,
      )
    }
  }
}
