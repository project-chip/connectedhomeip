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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class AccessControlClusterAccessRestrictionEntryStruct(
  val endpoint: UInt,
  val cluster: ULong,
  val restrictions: List<AccessControlClusterAccessRestrictionStruct>,
  val fabricIndex: UInt,
) {
  override fun toString(): String = buildString {
    append("AccessControlClusterAccessRestrictionEntryStruct {\n")
    append("\tendpoint : $endpoint\n")
    append("\tcluster : $cluster\n")
    append("\trestrictions : $restrictions\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_ENDPOINT), endpoint)
      put(ContextSpecificTag(TAG_CLUSTER), cluster)
      startArray(ContextSpecificTag(TAG_RESTRICTIONS))
      for (item in restrictions.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
      endStructure()
    }
  }

  companion object {
    private const val TAG_ENDPOINT = 0
    private const val TAG_CLUSTER = 1
    private const val TAG_RESTRICTIONS = 2
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): AccessControlClusterAccessRestrictionEntryStruct {
      tlvReader.enterStructure(tlvTag)
      val endpoint = tlvReader.getUInt(ContextSpecificTag(TAG_ENDPOINT))
      val cluster = tlvReader.getULong(ContextSpecificTag(TAG_CLUSTER))
      val restrictions =
        buildList<AccessControlClusterAccessRestrictionStruct> {
          tlvReader.enterArray(ContextSpecificTag(TAG_RESTRICTIONS))
          while (!tlvReader.isEndOfContainer()) {
            add(AccessControlClusterAccessRestrictionStruct.fromTlv(AnonymousTag, tlvReader))
          }
          tlvReader.exitContainer()
        }
      val fabricIndex = tlvReader.getUInt(ContextSpecificTag(TAG_FABRIC_INDEX))

      tlvReader.exitContainer()

      return AccessControlClusterAccessRestrictionEntryStruct(
        endpoint,
        cluster,
        restrictions,
        fabricIndex,
      )
    }
  }
}
