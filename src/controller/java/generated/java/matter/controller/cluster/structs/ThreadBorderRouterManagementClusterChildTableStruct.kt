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
package matter.controller.cluster.structs

import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ThreadBorderRouterManagementClusterChildTableStruct(
  val rloc16: UShort,
  val linkQuality: UByte,
  val routingRole: UByte
) {
  override fun toString(): String = buildString {
    append("ThreadBorderRouterManagementClusterChildTableStruct {\n")
    append("\trloc16 : $rloc16\n")
    append("\tlinkQuality : $linkQuality\n")
    append("\troutingRole : $routingRole\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_RLOC16), rloc16)
      put(ContextSpecificTag(TAG_LINK_QUALITY), linkQuality)
      put(ContextSpecificTag(TAG_ROUTING_ROLE), routingRole)
      endStructure()
    }
  }

  companion object {
    private const val TAG_RLOC16 = 0
    private const val TAG_LINK_QUALITY = 1
    private const val TAG_ROUTING_ROLE = 2

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader
    ): ThreadBorderRouterManagementClusterChildTableStruct {
      tlvReader.enterStructure(tlvTag)
      val rloc16 = tlvReader.getUShort(ContextSpecificTag(TAG_RLOC16))
      val linkQuality = tlvReader.getUByte(ContextSpecificTag(TAG_LINK_QUALITY))
      val routingRole = tlvReader.getUByte(ContextSpecificTag(TAG_ROUTING_ROLE))

      tlvReader.exitContainer()

      return ThreadBorderRouterManagementClusterChildTableStruct(rloc16, linkQuality, routingRole)
    }
  }
}
