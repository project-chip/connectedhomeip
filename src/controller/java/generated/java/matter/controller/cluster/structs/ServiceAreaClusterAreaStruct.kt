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

class ServiceAreaClusterAreaStruct(
  val areaID: UInt,
  val mapID: UInt?,
  val areaInfo: ServiceAreaClusterAreaInfoStruct,
) {
  override fun toString(): String = buildString {
    append("ServiceAreaClusterAreaStruct {\n")
    append("\tareaID : $areaID\n")
    append("\tmapID : $mapID\n")
    append("\tareaInfo : $areaInfo\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_AREA_ID), areaID)
      if (mapID != null) {
        put(ContextSpecificTag(TAG_MAP_ID), mapID)
      } else {
        putNull(ContextSpecificTag(TAG_MAP_ID))
      }
      areaInfo.toTlv(ContextSpecificTag(TAG_AREA_INFO), this)
      endStructure()
    }
  }

  companion object {
    private const val TAG_AREA_ID = 0
    private const val TAG_MAP_ID = 1
    private const val TAG_AREA_INFO = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ServiceAreaClusterAreaStruct {
      tlvReader.enterStructure(tlvTag)
      val areaID = tlvReader.getUInt(ContextSpecificTag(TAG_AREA_ID))
      val mapID =
        if (!tlvReader.isNull()) {
          tlvReader.getUInt(ContextSpecificTag(TAG_MAP_ID))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_MAP_ID))
          null
        }
      val areaInfo =
        ServiceAreaClusterAreaInfoStruct.fromTlv(ContextSpecificTag(TAG_AREA_INFO), tlvReader)

      tlvReader.exitContainer()

      return ServiceAreaClusterAreaStruct(areaID, mapID, areaInfo)
    }
  }
}
