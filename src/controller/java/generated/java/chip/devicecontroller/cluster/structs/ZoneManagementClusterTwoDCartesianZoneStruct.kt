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
import matter.tlv.TlvParsingException
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

import java.util.Optional

class ZoneManagementClusterTwoDCartesianZoneStruct (
    val name: String,
    val use: UInt,
    val vertices: List<ZoneManagementClusterTwoDCartesianVertexStruct>,
    val color: Optional<String>) {
  override fun toString(): String  = buildString {
    append("ZoneManagementClusterTwoDCartesianZoneStruct {\n")
    append("\tname : $name\n")
    append("\tuse : $use\n")
    append("\tvertices : $vertices\n")
    append("\tcolor : $color\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_NAME), name)
      put(ContextSpecificTag(TAG_USE), use)
      startArray(ContextSpecificTag(TAG_VERTICES))
      for (item in vertices.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
      if (color.isPresent) {
      val optcolor = color.get()
      put(ContextSpecificTag(TAG_COLOR), optcolor)
    }
      endStructure()
    }
  }

  companion object {
    private const val TAG_NAME = 0
    private const val TAG_USE = 1
    private const val TAG_VERTICES = 2
    private const val TAG_COLOR = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : ZoneManagementClusterTwoDCartesianZoneStruct {
      tlvReader.enterStructure(tlvTag)
      val name = tlvReader.getString(ContextSpecificTag(TAG_NAME))
      val use = tlvReader.getUInt(ContextSpecificTag(TAG_USE))
      val vertices = buildList<ZoneManagementClusterTwoDCartesianVertexStruct> {
      tlvReader.enterArray(ContextSpecificTag(TAG_VERTICES))
      while(!tlvReader.isEndOfContainer()) {
        add(ZoneManagementClusterTwoDCartesianVertexStruct.fromTlv(AnonymousTag, tlvReader))
      }
      tlvReader.exitContainer()
    }
      val color = if (tlvReader.isNextTag(ContextSpecificTag(TAG_COLOR))) {
      Optional.of(tlvReader.getString(ContextSpecificTag(TAG_COLOR)))
    } else {
      Optional.empty()
    }
      
      tlvReader.exitContainer()

      return ZoneManagementClusterTwoDCartesianZoneStruct(name, use, vertices, color)
    }
  }
}
