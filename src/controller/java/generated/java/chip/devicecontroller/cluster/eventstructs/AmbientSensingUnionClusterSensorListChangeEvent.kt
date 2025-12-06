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
import matter.tlv.TlvParsingException
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

import java.util.Optional

class AmbientSensingUnionClusterSensorListChangeEvent (
    val unionSensorList: List<UInt>) {
  override fun toString(): String  = buildString {
    append("AmbientSensingUnionClusterSensorListChangeEvent {\n")
    append("\tunionSensorList : $unionSensorList\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      startArray(ContextSpecificTag(TAG_UNION_SENSOR_LIST))
      for (item in unionSensorList.iterator()) {
        put(AnonymousTag, item)
      }
      endArray()
      endStructure()
    }
  }

  companion object {
    private const val TAG_UNION_SENSOR_LIST = 0

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : AmbientSensingUnionClusterSensorListChangeEvent {
      tlvReader.enterStructure(tlvTag)
      val unionSensorList = buildList <UInt> {
      tlvReader.enterArray(ContextSpecificTag(TAG_UNION_SENSOR_LIST))
      while(!tlvReader.isEndOfContainer()) {
        this.add(tlvReader.getUInt(AnonymousTag))
      }
      tlvReader.exitContainer()
    }
      
      tlvReader.exitContainer()

      return AmbientSensingUnionClusterSensorListChangeEvent(unionSensorList)
    }
  }
}
