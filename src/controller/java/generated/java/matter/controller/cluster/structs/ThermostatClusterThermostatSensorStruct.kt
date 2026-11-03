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

import java.util.Optional
import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ThermostatClusterThermostatSensorStruct(
  val name: String,
  val sensorHandle: ByteArray,
  val cluster: UInt,
  val endpoint: Optional<UShort>,
  val node: Optional<ULong>,
  val fabricIndex: Optional<UByte>,
) {
  override fun toString(): String = buildString {
    append("ThermostatClusterThermostatSensorStruct {\n")
    append("\tname : $name\n")
    append("\tsensorHandle : $sensorHandle\n")
    append("\tcluster : $cluster\n")
    append("\tendpoint : $endpoint\n")
    append("\tnode : $node\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_NAME), name)
      put(ContextSpecificTag(TAG_SENSOR_HANDLE), sensorHandle)
      put(ContextSpecificTag(TAG_CLUSTER), cluster)
      if (endpoint.isPresent) {
        val optendpoint = endpoint.get()
        put(ContextSpecificTag(TAG_ENDPOINT), optendpoint)
      }
      if (node.isPresent) {
        val optnode = node.get()
        put(ContextSpecificTag(TAG_NODE), optnode)
      }
      if (fabricIndex.isPresent) {
        val optfabricIndex = fabricIndex.get()
        put(ContextSpecificTag(TAG_FABRIC_INDEX), optfabricIndex)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_NAME = 0
    private const val TAG_SENSOR_HANDLE = 1
    private const val TAG_CLUSTER = 2
    private const val TAG_ENDPOINT = 3
    private const val TAG_NODE = 4
    private const val TAG_FABRIC_INDEX = 5

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ThermostatClusterThermostatSensorStruct {
      tlvReader.enterStructure(tlvTag)
      val name = tlvReader.getString(ContextSpecificTag(TAG_NAME))
      val sensorHandle = tlvReader.getByteArray(ContextSpecificTag(TAG_SENSOR_HANDLE))
      val cluster = tlvReader.getUInt(ContextSpecificTag(TAG_CLUSTER))
      val endpoint =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_ENDPOINT))) {
          Optional.of(tlvReader.getUShort(ContextSpecificTag(TAG_ENDPOINT)))
        } else {
          Optional.empty()
        }
      val node =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_NODE))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_NODE)))
        } else {
          Optional.empty()
        }
      val fabricIndex =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_FABRIC_INDEX))) {
          Optional.of(tlvReader.getUByte(ContextSpecificTag(TAG_FABRIC_INDEX)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ThermostatClusterThermostatSensorStruct(
        name,
        sensorHandle,
        cluster,
        endpoint,
        node,
        fabricIndex,
      )
    }
  }
}
