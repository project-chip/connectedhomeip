/*
 *   Copyright (c) 2026 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package com.google.chip.chiptool.devicemanagement

import com.google.chip.chiptool.util.toAny
import com.google.gson.JsonArray
import com.google.gson.JsonElement
import com.google.gson.JsonNull
import com.google.gson.JsonObject
import com.google.gson.JsonPrimitive
import java.util.Base64
import java.util.Locale
import matter.jsontlv.fromJsonString
import matter.tlv.AnonymousTag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

/** Result of decoding a Matter TLV attribute report. */
data class DecodedAttributeValue(
  val rawValue: Any?,
  val formattedValue: String,
  val unit: String? = null,
  val attributeMeta: MatterAttributeMeta? = null,
) {
  val value: Any?
    get() = rawValue
}

/**
 * Universal Dynamic Parameter Encoder and Decoder. Serializes arbitrary Matter command parameters
 * and attribute writes into `matter.jsontlv` JSON strings and Matter TLV bytes, and decodes Matter
 * IM response streams into structured typed values and human-readable text.
 */
object UniversalTlvEncoder {

  /**
   * Encodes command arguments for a specific Matter command into a `matter.jsontlv` JSON string.
   *
   * @param commandMeta Metadata defining the command schema and parameter fields.
   * @param params Key-value map of parameter names/aliases to values.
   * @return `matter.jsontlv` JSON string.
   */
  fun encodeCommandJson(
    commandMeta: MatterCommandMeta,
    params: Map<String, Any?> = emptyMap(),
  ): String {
    validateCommandParams(commandMeta, params)
    val root = JsonObject()

    for (field in commandMeta.requestFields) {
      val res = encodeCommandField(field, params)
      if (res != null) {
        root.add(res.first, res.second)
      }
    }

    return root.toString()
  }

  /**
   * Encodes command arguments for a specific Matter command into a top-level TLV Structure using
   * the SDK's [TlvWriter.fromJsonString] converter.
   */
  fun encodeCommandPayload(
    commandMeta: MatterCommandMeta,
    params: Map<String, Any?> = emptyMap(),
  ): ByteArray = TlvWriter().fromJsonString(encodeCommandJson(commandMeta, params))

  private fun validateCommandParams(commandMeta: MatterCommandMeta, params: Map<String, Any?>) {
    for (field in commandMeta.requestFields) {
      val rawValue = findParamValue(params, field.name, field.tagId) ?: field.defaultValue
      if (rawValue == null && !field.isOptional && !field.isNullable) {
        val msg =
          "Missing mandatory parameter '${field.name}' " +
            "(tag ${field.tagId}) for command ${commandMeta.name}"
        throw IllegalArgumentException(msg)
      }
    }
  }

  private fun encodeCommandField(
    field: MatterFieldMeta,
    params: Map<String, Any?>,
  ): Pair<String, JsonElement>? {
    val hasKey = hasParamKey(params, field.name, field.tagId)
    val rawValue = findParamValue(params, field.name, field.tagId) ?: field.defaultValue
    val typeStr = getTypeString(field.type)
    val key = "${field.tagId}:$typeStr"

    if (rawValue == null) {
      if (hasKey && field.isNullable) {
        return "${field.tagId}:NULL" to JsonNull.INSTANCE
      } else if (!hasKey && !field.isOptional && field.isNullable) {
        return "${field.tagId}:NULL" to JsonNull.INSTANCE
      }
      return null
    }
    return encodeField(key, field.type, rawValue, field)
  }

  /**
   * Encodes an attribute value into a `matter.jsontlv` JSON string for an Interaction Model Write
   * Request.
   */
  fun encodeAttributeWriteJson(attributeMeta: MatterAttributeMeta, value: Any?): String {
    val root = JsonObject()
    if (value == null) {
      root.add("0:NULL", JsonNull.INSTANCE)
    } else {
      val fieldMeta =
        MatterFieldMeta(
          tagId = 0,
          name = attributeMeta.name,
          type = attributeMeta.type,
          isNullable = attributeMeta.isNullable,
          enumValues = attributeMeta.enumValues,
        )
      val res =
        encodeField("0:${getTypeString(attributeMeta.type)}", attributeMeta.type, value, fieldMeta)
      root.add(res.first, res.second)
    }
    return root.toString()
  }

  /** Encodes an attribute value into TLV bytes for an Interaction Model Write Request. */
  fun encodeAttributeWritePayload(attributeMeta: MatterAttributeMeta, value: Any?): ByteArray {
    val structBytes = TlvWriter().fromJsonString(encodeAttributeWriteJson(attributeMeta, value))
    val reader = TlvReader(structBytes)
    reader.enterStructure(AnonymousTag)
    return TlvWriter().copyElement(AnonymousTag, reader).validateTlv().getEncoded()
  }

  /**
   * Decodes an attribute report TLV byte buffer into structured and human-readable representations
   * using [TlvReader.toAny].
   */
  fun decodeAttributeReport(
    attributeMeta: MatterAttributeMeta,
    tlvBytes: ByteArray,
  ): DecodedAttributeValue {
    val parsed = runCatching { TlvReader(tlvBytes).toAny() }.getOrNull()
    val rawValue =
      if (attributeMeta.type == MatterDataType.UINT64 && parsed is Long) {
        parsed.toULong().toString()
      } else {
        parsed
      }
    val formatted = formatDecodedValue(rawValue, attributeMeta)
    return DecodedAttributeValue(
      rawValue = rawValue,
      formattedValue = formatted,
      unit = attributeMeta.unit,
      attributeMeta = attributeMeta,
    )
  }

  // --- Internal Field Encoding ---

  private fun getTypeString(type: MatterDataType): String {
    return when (type) {
      MatterDataType.BOOLEAN -> "BOOL"
      MatterDataType.INT8,
      MatterDataType.INT16,
      MatterDataType.INT32,
      MatterDataType.INT64 -> "INT"
      MatterDataType.UINT8,
      MatterDataType.UINT16,
      MatterDataType.UINT32,
      MatterDataType.UINT64,
      MatterDataType.ENUM8,
      MatterDataType.ENUM16,
      MatterDataType.BITMAP8,
      MatterDataType.BITMAP16,
      MatterDataType.BITMAP32 -> "UINT"
      MatterDataType.FLOAT32 -> "FLOAT"
      MatterDataType.FLOAT64 -> "DOUBLE"
      MatterDataType.UTF8_STRING -> "STRING"
      MatterDataType.OCTET_STRING -> "BYTES"
      MatterDataType.ARRAY -> "ARRAY-UINT"
      MatterDataType.STRUCT -> "STRUCT"
      MatterDataType.NULL -> "NULL"
    }
  }

  private fun encodeField(
    key: String,
    type: MatterDataType,
    value: Any,
    fieldMeta: MatterFieldMeta?,
  ): Pair<String, JsonElement> {
    return when (type) {
      MatterDataType.BOOLEAN -> {
        val b =
          when (value) {
            is Boolean -> value
            is Number -> value.toInt() != 0
            is String ->
              value.equals("true", ignoreCase = true) ||
                value == "1" ||
                value.equals("on", ignoreCase = true)
            else -> false
          }
        key to JsonPrimitive(b)
      }
      MatterDataType.INT8,
      MatterDataType.INT16,
      MatterDataType.INT32,
      MatterDataType.INT64 -> {
        key to JsonPrimitive(coerceToLong(value, fieldMeta))
      }
      MatterDataType.UINT8,
      MatterDataType.UINT16,
      MatterDataType.UINT32,
      MatterDataType.UINT64,
      MatterDataType.ENUM8,
      MatterDataType.ENUM16,
      MatterDataType.BITMAP8,
      MatterDataType.BITMAP16,
      MatterDataType.BITMAP32 -> {
        val num = coerceToULong(value, fieldMeta).toLong()
        key to JsonPrimitive(num)
      }
      MatterDataType.FLOAT32 -> {
        val f =
          when (value) {
            is Number -> value.toFloat()
            is String -> value.toFloatOrNull() ?: 0f
            else -> 0f
          }
        key to JsonPrimitive(f)
      }
      MatterDataType.FLOAT64 -> {
        val d =
          when (value) {
            is Number -> value.toDouble()
            is String -> value.toDoubleOrNull() ?: 0.0
            else -> 0.0
          }
        key to JsonPrimitive(d)
      }
      MatterDataType.UTF8_STRING -> {
        key to JsonPrimitive(value.toString())
      }
      MatterDataType.OCTET_STRING -> {
        val bytes =
          when (value) {
            is ByteArray -> value
            is String -> hexStringToByteArray(value)
            else -> ByteArray(0)
          }
        key to JsonPrimitive(Base64.getEncoder().encodeToString(bytes))
      }
      MatterDataType.ARRAY -> {
        val list =
          when (value) {
            is Collection<*> -> value.toList()
            is Array<*> -> value.toList()
            else -> listOf(value)
          }
        val arr = JsonArray()
        for (item in list.filterNotNull()) {
          val (_, v) = encodeField("0:UINT", MatterDataType.UINT32, item, null)
          arr.add(v)
        }
        key to arr
      }
      MatterDataType.STRUCT -> {
        val map = value as? Map<*, *>
        val obj = JsonObject()
        if (map != null) {
          for ((k, v) in map) {
            val childValue = v ?: continue
            val childTagNum = k?.toString()?.toIntOrNull() ?: 0
            val (childKey, cv) =
              encodeField("$childTagNum:UINT", MatterDataType.UINT32, childValue, null)
            obj.add(childKey, cv)
          }
        }
        key to obj
      }
      MatterDataType.NULL -> {
        key.substringBeforeLast(":") + ":NULL" to JsonNull.INSTANCE
      }
    }
  }

  fun formatDecodedValue(rawValue: Any?, attributeMeta: MatterAttributeMeta): String {
    if (rawValue == null) return "Unknown"

    // Enum mapping check
    if (attributeMeta.enumValues.isNotEmpty()) {
      val longVal = (rawValue as? Number)?.toLong()
      val matchedName = attributeMeta.enumValues.entries.firstOrNull { it.value == longVal }?.key
      if (matchedName != null) return matchedName
    }

    // Special cluster attribute units
    return when (attributeMeta.name) {
      "LocalTemperature",
      "OccupiedHeatingSetpoint",
      "OccupiedCoolingSetpoint" -> {
        val centidegrees = (rawValue as? Number)?.toShort() ?: 0
        String.format(Locale.US, "%.1f°C", centidegrees / 100.0)
      }
      "MeasuredValue",
      "MinMeasuredValue",
      "MaxMeasuredValue",
      "Tolerance" -> {
        when (attributeMeta.unit) {
          "°C" -> {
            val centidegrees = (rawValue as? Number)?.toShort() ?: 0
            String.format(Locale.US, "%.1f°C", centidegrees / 100.0)
          }
          "%" -> {
            val centiPercent = (rawValue as? Number)?.toLong() ?: 0
            String.format(Locale.US, "%.1f%%", centiPercent / 100.0)
          }
          // Illuminance is 10000 * log10(lux) + 1; 0 means too low to measure.
          "lx" -> {
            val raw = (rawValue as? Number)?.toInt() ?: 0
            if (raw <= 0) "<1 lx"
            else String.format(Locale.US, "%.0f lx", Math.pow(10.0, (raw - 1) / 10000.0))
          }
          else -> rawValue.toString()
        }
      }
      "ColorTemperatureMireds" -> {
        val mireds = (rawValue as? Number)?.toInt() ?: 0
        if (mireds > 0) {
          val kelvin = MatterClusterMetaRegistry.miredsToKelvin(mireds)
          "$kelvin K ($mireds Mireds)"
        } else "$mireds Mireds"
      }
      "OnOff" -> {
        if (rawValue == true) "ON" else "OFF"
      }
      "CurrentLevel" -> {
        val lvl = (rawValue as? Number)?.toInt()?.coerceIn(0, 254)
        if (lvl != null) {
          "${MatterClusterMetaRegistry.levelToPercentage(lvl.toUByte()).toInt()}%"
        } else {
          rawValue.toString()
        }
      }
      "PercentSetting",
      "PercentCurrent" -> {
        "$rawValue%"
      }
      "BatPercentRemaining" -> {
        val rawNum = (rawValue as? Number) ?: 0
        "${MatterClusterMetaRegistry.batHalfPercentToPercent(rawNum)}%"
      }
      "LockState" -> {
        when ((rawValue as? Number)?.toInt()) {
          1 -> "Locked"
          2 -> "Unlocked"
          else -> "Not Fully Locked"
        }
      }
      "StateValue" -> {
        if (rawValue == true) "Open / Active" else "Closed / Normal"
      }
      else -> {
        if (attributeMeta.unit == "°C" && rawValue is Number) {
          String.format(Locale.US, "%.1f°C", rawValue.toShort() / 100.0)
        } else if (attributeMeta.unit != null) {
          "$rawValue ${attributeMeta.unit}"
        } else {
          rawValue.toString()
        }
      }
    }
  }

  // --- Helper Utilities ---

  private fun hasParamKey(params: Map<String, Any?>, name: String, tagId: Int): Boolean {
    if (params.containsKey(name)) return true
    val lowerName = name.lowercase()
    for (k in params.keys) {
      if (k.lowercase() == lowerName) return true
    }
    val tagKey = "tag$tagId"
    return params.containsKey(tagKey) || params.containsKey(tagId.toString())
  }

  private fun findParamValue(params: Map<String, Any?>, name: String, tagId: Int): Any? {
    if (params.containsKey(name)) return params[name]
    val lowerName = name.lowercase()
    for ((k, v) in params) {
      if (k.lowercase() == lowerName) return v
    }
    val tagKey = "tag$tagId"
    if (params.containsKey(tagKey)) return params[tagKey]
    if (params.containsKey(tagId.toString())) return params[tagId.toString()]
    return null
  }

  private fun coerceToLong(value: Any, fieldMeta: MatterFieldMeta?): Long {
    return when (value) {
      is Number -> value.toLong()
      is Boolean -> if (value) 1L else 0L
      is String -> {
        // Check enum mapping
        fieldMeta?.enumValues?.get(value)?.let {
          return it
        }
        value.toDoubleOrNull()?.toLong() ?: 0L
      }
      else -> 0L
    }
  }

  private fun coerceToULong(value: Any, fieldMeta: MatterFieldMeta?): ULong {
    return when (value) {
      is Number -> value.toLong().toULong()
      is Boolean -> if (value) 1uL else 0uL
      is String -> {
        fieldMeta?.enumValues?.get(value)?.let {
          return it.toULong()
        }
        value.toDoubleOrNull()?.toLong()?.toULong() ?: 0uL
      }
      else -> 0uL
    }
  }

  private fun hexStringToByteArray(hex: String): ByteArray {
    val stripped = hex.replace(" ", "").replace("0x", "", ignoreCase = true)
    if (stripped.isEmpty()) return ByteArray(0)
    val clean = if (stripped.length % 2 != 0) "0$stripped" else stripped
    val len = clean.length
    val data = ByteArray(len / 2)
    var i = 0
    while (i < len) {
      val hi = Character.digit(clean[i], 16).coerceAtLeast(0)
      val lo = Character.digit(clean[i + 1], 16).coerceAtLeast(0)
      data[i / 2] = ((hi shl 4) + lo).toByte()
      i += 2
    }
    return data
  }
}
