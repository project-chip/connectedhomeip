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

class ElectricalPowerMeasurementClusterMeasurementRangeStruct(
  val measurementType: UShort,
  val min: Long,
  val max: Long,
  val startTimestamp: Optional<UInt>,
  val endTimestamp: Optional<UInt>,
  val minTimestamp: Optional<UInt>,
  val maxTimestamp: Optional<UInt>,
  val startSystime: Optional<ULong>,
  val endSystime: Optional<ULong>,
  val minSystime: Optional<ULong>,
  val maxSystime: Optional<ULong>,
) {
  override fun toString(): String = buildString {
    append("ElectricalPowerMeasurementClusterMeasurementRangeStruct {\n")
    append("\tmeasurementType : $measurementType\n")
    append("\tmin : $min\n")
    append("\tmax : $max\n")
    append("\tstartTimestamp : $startTimestamp\n")
    append("\tendTimestamp : $endTimestamp\n")
    append("\tminTimestamp : $minTimestamp\n")
    append("\tmaxTimestamp : $maxTimestamp\n")
    append("\tstartSystime : $startSystime\n")
    append("\tendSystime : $endSystime\n")
    append("\tminSystime : $minSystime\n")
    append("\tmaxSystime : $maxSystime\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_MEASUREMENT_TYPE), measurementType)
      put(ContextSpecificTag(TAG_MIN), min)
      put(ContextSpecificTag(TAG_MAX), max)
      if (startTimestamp.isPresent) {
        val optstartTimestamp = startTimestamp.get()
        put(ContextSpecificTag(TAG_START_TIMESTAMP), optstartTimestamp)
      }
      if (endTimestamp.isPresent) {
        val optendTimestamp = endTimestamp.get()
        put(ContextSpecificTag(TAG_END_TIMESTAMP), optendTimestamp)
      }
      if (minTimestamp.isPresent) {
        val optminTimestamp = minTimestamp.get()
        put(ContextSpecificTag(TAG_MIN_TIMESTAMP), optminTimestamp)
      }
      if (maxTimestamp.isPresent) {
        val optmaxTimestamp = maxTimestamp.get()
        put(ContextSpecificTag(TAG_MAX_TIMESTAMP), optmaxTimestamp)
      }
      if (startSystime.isPresent) {
        val optstartSystime = startSystime.get()
        put(ContextSpecificTag(TAG_START_SYSTIME), optstartSystime)
      }
      if (endSystime.isPresent) {
        val optendSystime = endSystime.get()
        put(ContextSpecificTag(TAG_END_SYSTIME), optendSystime)
      }
      if (minSystime.isPresent) {
        val optminSystime = minSystime.get()
        put(ContextSpecificTag(TAG_MIN_SYSTIME), optminSystime)
      }
      if (maxSystime.isPresent) {
        val optmaxSystime = maxSystime.get()
        put(ContextSpecificTag(TAG_MAX_SYSTIME), optmaxSystime)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_MEASUREMENT_TYPE = 0
    private const val TAG_MIN = 1
    private const val TAG_MAX = 2
    private const val TAG_START_TIMESTAMP = 3
    private const val TAG_END_TIMESTAMP = 4
    private const val TAG_MIN_TIMESTAMP = 5
    private const val TAG_MAX_TIMESTAMP = 6
    private const val TAG_START_SYSTIME = 7
    private const val TAG_END_SYSTIME = 8
    private const val TAG_MIN_SYSTIME = 9
    private const val TAG_MAX_SYSTIME = 10

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): ElectricalPowerMeasurementClusterMeasurementRangeStruct {
      tlvReader.enterStructure(tlvTag)
      val measurementType = tlvReader.getUShort(ContextSpecificTag(TAG_MEASUREMENT_TYPE))
      val min = tlvReader.getLong(ContextSpecificTag(TAG_MIN))
      val max = tlvReader.getLong(ContextSpecificTag(TAG_MAX))
      val startTimestamp =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_START_TIMESTAMP))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_START_TIMESTAMP)))
        } else {
          Optional.empty()
        }
      val endTimestamp =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_END_TIMESTAMP))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_END_TIMESTAMP)))
        } else {
          Optional.empty()
        }
      val minTimestamp =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MIN_TIMESTAMP))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_MIN_TIMESTAMP)))
        } else {
          Optional.empty()
        }
      val maxTimestamp =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MAX_TIMESTAMP))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_MAX_TIMESTAMP)))
        } else {
          Optional.empty()
        }
      val startSystime =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_START_SYSTIME))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_START_SYSTIME)))
        } else {
          Optional.empty()
        }
      val endSystime =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_END_SYSTIME))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_END_SYSTIME)))
        } else {
          Optional.empty()
        }
      val minSystime =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MIN_SYSTIME))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_MIN_SYSTIME)))
        } else {
          Optional.empty()
        }
      val maxSystime =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MAX_SYSTIME))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_MAX_SYSTIME)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ElectricalPowerMeasurementClusterMeasurementRangeStruct(
        measurementType,
        min,
        max,
        startTimestamp,
        endTimestamp,
        minTimestamp,
        maxTimestamp,
        startSystime,
        endSystime,
        minSystime,
        maxSystime,
      )
    }
  }
}
