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
import java.util.Optional
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class CameraAvStreamManagementClusterVideoSensorParamsStruct(
  val sensorWidth: UInt,
  val sensorHeight: UInt,
  val maxFPS: UInt,
  val maxHDRFPS: Optional<UInt>,
) {
  override fun toString(): String = buildString {
    append("CameraAvStreamManagementClusterVideoSensorParamsStruct {\n")
    append("\tsensorWidth : $sensorWidth\n")
    append("\tsensorHeight : $sensorHeight\n")
    append("\tmaxFPS : $maxFPS\n")
    append("\tmaxHDRFPS : $maxHDRFPS\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_SENSOR_WIDTH), sensorWidth)
      put(ContextSpecificTag(TAG_SENSOR_HEIGHT), sensorHeight)
      put(ContextSpecificTag(TAG_MAX_FPS), maxFPS)
      if (maxHDRFPS.isPresent) {
        val optmaxHDRFPS = maxHDRFPS.get()
        put(ContextSpecificTag(TAG_MAX_HDRFPS), optmaxHDRFPS)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_SENSOR_WIDTH = 0
    private const val TAG_SENSOR_HEIGHT = 1
    private const val TAG_MAX_FPS = 2
    private const val TAG_MAX_HDRFPS = 3

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): CameraAvStreamManagementClusterVideoSensorParamsStruct {
      tlvReader.enterStructure(tlvTag)
      val sensorWidth = tlvReader.getUInt(ContextSpecificTag(TAG_SENSOR_WIDTH))
      val sensorHeight = tlvReader.getUInt(ContextSpecificTag(TAG_SENSOR_HEIGHT))
      val maxFPS = tlvReader.getUInt(ContextSpecificTag(TAG_MAX_FPS))
      val maxHDRFPS =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MAX_HDRFPS))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_MAX_HDRFPS)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return CameraAvStreamManagementClusterVideoSensorParamsStruct(
        sensorWidth,
        sensorHeight,
        maxFPS,
        maxHDRFPS,
      )
    }
  }
}
