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

class ProximityRangingClusterRangingMeasurementDataStruct(
  val wiFiDevIK: Optional<ByteArray>,
  val BLEDeviceId: Optional<ULong>,
  val BLTDevIK: Optional<ByteArray>,
  val timeOfMeasurement: Optional<UInt>,
  val distance: UShort?,
  val accuracy: Optional<Short>,
  val rdr: Optional<ProximityRangingClusterRDRStruct>,
  val rssi: Optional<Byte>?,
  val txPower: Optional<Byte>?,
) {
  override fun toString(): String = buildString {
    append("ProximityRangingClusterRangingMeasurementDataStruct {\n")
    append("\twiFiDevIK : $wiFiDevIK\n")
    append("\tBLEDeviceId : $BLEDeviceId\n")
    append("\tBLTDevIK : $BLTDevIK\n")
    append("\ttimeOfMeasurement : $timeOfMeasurement\n")
    append("\tdistance : $distance\n")
    append("\taccuracy : $accuracy\n")
    append("\trdr : $rdr\n")
    append("\trssi : $rssi\n")
    append("\ttxPower : $txPower\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (wiFiDevIK.isPresent) {
        val optwiFiDevIK = wiFiDevIK.get()
        put(ContextSpecificTag(TAG_WI_FI_DEV_IK), optwiFiDevIK)
      }
      if (BLEDeviceId.isPresent) {
        val optBLEDeviceId = BLEDeviceId.get()
        put(ContextSpecificTag(TAG_BLE_DEVICE_ID), optBLEDeviceId)
      }
      if (BLTDevIK.isPresent) {
        val optBLTDevIK = BLTDevIK.get()
        put(ContextSpecificTag(TAG_BLT_DEV_IK), optBLTDevIK)
      }
      if (timeOfMeasurement.isPresent) {
        val opttimeOfMeasurement = timeOfMeasurement.get()
        put(ContextSpecificTag(TAG_TIME_OF_MEASUREMENT), opttimeOfMeasurement)
      }
      if (distance != null) {
        put(ContextSpecificTag(TAG_DISTANCE), distance)
      } else {
        putNull(ContextSpecificTag(TAG_DISTANCE))
      }
      if (accuracy.isPresent) {
        val optaccuracy = accuracy.get()
        put(ContextSpecificTag(TAG_ACCURACY), optaccuracy)
      }
      if (rdr.isPresent) {
        val optrdr = rdr.get()
        optrdr.toTlv(ContextSpecificTag(TAG_RDR), this)
      }
      if (rssi != null) {
        if (rssi.isPresent) {
          val optrssi = rssi.get()
          put(ContextSpecificTag(TAG_RSSI), optrssi)
        }
      } else {
        putNull(ContextSpecificTag(TAG_RSSI))
      }
      if (txPower != null) {
        if (txPower.isPresent) {
          val opttxPower = txPower.get()
          put(ContextSpecificTag(TAG_TX_POWER), opttxPower)
        }
      } else {
        putNull(ContextSpecificTag(TAG_TX_POWER))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_WI_FI_DEV_IK = 0
    private const val TAG_BLE_DEVICE_ID = 1
    private const val TAG_BLT_DEV_IK = 2
    private const val TAG_TIME_OF_MEASUREMENT = 3
    private const val TAG_DISTANCE = 4
    private const val TAG_ACCURACY = 5
    private const val TAG_RDR = 6
    private const val TAG_RSSI = 7
    private const val TAG_TX_POWER = 8

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): ProximityRangingClusterRangingMeasurementDataStruct {
      tlvReader.enterStructure(tlvTag)
      val wiFiDevIK =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_WI_FI_DEV_IK))) {
          Optional.of(tlvReader.getByteArray(ContextSpecificTag(TAG_WI_FI_DEV_IK)))
        } else {
          Optional.empty()
        }
      val BLEDeviceId =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_BLE_DEVICE_ID))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_BLE_DEVICE_ID)))
        } else {
          Optional.empty()
        }
      val BLTDevIK =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_BLT_DEV_IK))) {
          Optional.of(tlvReader.getByteArray(ContextSpecificTag(TAG_BLT_DEV_IK)))
        } else {
          Optional.empty()
        }
      val timeOfMeasurement =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_TIME_OF_MEASUREMENT))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_TIME_OF_MEASUREMENT)))
        } else {
          Optional.empty()
        }
      val distance =
        if (!tlvReader.isNull()) {
          tlvReader.getUShort(ContextSpecificTag(TAG_DISTANCE))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_DISTANCE))
          null
        }
      val accuracy =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_ACCURACY))) {
          Optional.of(tlvReader.getShort(ContextSpecificTag(TAG_ACCURACY)))
        } else {
          Optional.empty()
        }
      val rdr =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_RDR))) {
          Optional.of(
            ProximityRangingClusterRDRStruct.fromTlv(ContextSpecificTag(TAG_RDR), tlvReader)
          )
        } else {
          Optional.empty()
        }
      val rssi =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_RSSI))) {
            Optional.of(tlvReader.getByte(ContextSpecificTag(TAG_RSSI)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_RSSI))
          null
        }
      val txPower =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_TX_POWER))) {
            Optional.of(tlvReader.getByte(ContextSpecificTag(TAG_TX_POWER)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_TX_POWER))
          null
        }

      tlvReader.exitContainer()

      return ProximityRangingClusterRangingMeasurementDataStruct(
        wiFiDevIK,
        BLEDeviceId,
        BLTDevIK,
        timeOfMeasurement,
        distance,
        accuracy,
        rdr,
        rssi,
        txPower,
      )
    }
  }
}
