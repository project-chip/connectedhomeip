/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    Copyright (c) 2023 Google LLC.
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

package matter.jsontlv

import com.google.gson.JsonArray
import com.google.gson.JsonElement
import com.google.gson.JsonObject
import com.google.gson.JsonParser
import java.util.Base64
import matter.tlv.*

/**
 * Implements Matter JSON to TLV converter.
 *
 * Note that NOT all TLV configurations are supported by the current implementation. Below is the
 * list of limitations:
 * - TLV Lists are not supported
 * - Multi-Dimensional TLV Arrays are not supported
 * - All elements of an array MUST be of the same type
 * - The top level TLV element MUST be a single structure with AnonymousTag
 * - The following tags are supported:
 *     - AnonymousTag are used only with TLV Arrays elements or a top-level structure
 *     - ContextSpecificTag are used only with TLV Structure elements
 *     - CommonProfileTag are used only with TLV Structure elements
 * - Infinity Float/Double values are not supported
 *
 * @param json string representing Json encoded data to be converted into TLV format
 * @throws IllegalArgumentException if the data was invalid
 */
fun TlvWriter.fromJsonString(json: String): ByteArray {
  return putJsonString(AnonymousTag, json).validateTlv().getEncoded()
}

/**
 * Converts Json string into TLV writer object.
 *
 * @param tag the TLV tag to be encoded.
 * @param json String representing Json to be converted to TLV.
 * @throws IllegalArgumentException if the data was invalid
 */
fun TlvWriter.putJsonString(tag: Tag, json: String): TlvWriter {
  validateIsJsonObjectAndConvert(JsonParser.parseString(json), tag)
  return this
}

/**
 * Converts Json Object into TLV Structure or TLV top level elements.
 *
 * @param json object to be converted to TLV.
 * @throws IllegalArgumentException if the data was invalid
 */
private fun TlvWriter.fromJson(json: JsonObject): TlvWriter {
  json.keySet().forEach { key ->
    val (tag, type, subType) = extractTagAndTypeFromJsonKey(key)
    fromJson(json.get(key), tag, type, subType)
  }
  return this
}

/**
 * Converts Json Array into TLV Array.
 *
 * @param json object to be converted to TLV.
 * @param type Type of array elements.
 * @throws IllegalArgumentException if the data was invalid
 */
private fun TlvWriter.fromJson(json: JsonArray, type: String): TlvWriter {
  json.iterator().forEach { element -> fromJson(element, AnonymousTag, type) }
  return this
}

/**
 * Converts Json Element into TLV Array.
 *
 * @param element element to be converted to TLV.
 * @param tag element tag.
 * @param type element type.
 * @param subType array elements type. Only relevant when type is an Array. Should be empty string
 *   in all other cases.
 * @throws IllegalArgumentException if the data was invalid
 */
private fun TlvWriter.fromJson(element: JsonElement, tag: Tag, type: String, subType: String = "") {
  when (type) {
    JSON_VALUE_TYPE_INT -> put(tag, validateIsNumber(element).toLong())
    JSON_VALUE_TYPE_UINT -> put(tag, validateIsNumber(element).toLong().toULong())
    JSON_VALUE_TYPE_BOOL -> put(tag, validateIsBoolean(element))
    JSON_VALUE_TYPE_FLOAT -> put(tag, validateIsDouble(element).toFloat())
    JSON_VALUE_TYPE_DOUBLE -> put(tag, validateIsDouble(element))
    JSON_VALUE_TYPE_BYTES -> put(tag, validateIsString(element).base64Encode())
    JSON_VALUE_TYPE_STRING -> put(tag, validateIsString(element))
    JSON_VALUE_TYPE_NULL -> validateIsNullAndPut(element, tag)
    JSON_VALUE_TYPE_STRUCT -> validateIsJsonObjectAndConvert(element, tag)
    JSON_VALUE_TYPE_ARRAY -> {
      if (subType.isEmpty()) {
        throw IllegalArgumentException("Multi-Dimensional JSON Array is Invalid")
      } else {
        require(element.isJsonArray()) { "Expected Array; the actual element is: $element" }
        startArray(tag).fromJson(element.getAsJsonArray(), subType).endArray()
      }
    }
    JSON_VALUE_TYPE_EMPTY ->
      throw IllegalArgumentException("Empty array was expected but there is value: $element}")
    else -> throw IllegalArgumentException("Invalid type was specified: $type")
  }
}

/**
 * Extracts tag and type fields from Json key. Valid JSON key SHOULD have 1, 2, or 3 fields
 * constracted as [name:][tag:]type[-subtype]
 *
 * @param key Json element key value.
 * @throws IllegalArgumentException if the key format was invalid
 */
private fun extractTagAndTypeFromJsonKey(key: String): Triple<Tag, String, String> {
  val keyFields = key.split(":")
  var type = keyFields.last()
  val typeFields = type.split("-")
  var subType = ""

  val tagNumber =
    when (keyFields.size) {
      2 -> keyFields.first().toUIntOrNull()
      3 -> keyFields[1].toUIntOrNull()
      else -> throw IllegalArgumentException("Invalid JSON key value: $key")
    }

  val tag =
    when {
      tagNumber == null -> throw IllegalArgumentException("Invalid JSON key value: $key")
      tagNumber <= UByte.MAX_VALUE.toUInt() -> ContextSpecificTag(tagNumber.toInt())
      tagNumber <= UShort.MAX_VALUE.toUInt() -> CommonProfileTag(2, tagNumber)
      else -> CommonProfileTag(4, tagNumber)
    }

  // Valid type field of the JSON key SHOULD have type and optional subtype component
  require(typeFields.size in (1..2)) { "Invalid JSON key value: $key" }

  if (typeFields.size == 2) {
    require(typeFields[0] == JSON_VALUE_TYPE_ARRAY) { "Invalid JSON key value: $key" }
    type = JSON_VALUE_TYPE_ARRAY
    subType = typeFields[1]
  }

  return Triple(tag, type, subType)
}

private fun String.base64Encode(): ByteArray {
  return Base64.getDecoder().decode(this)
}

/** Verifies JsonElement is Number. If yes, returns the value. */
private fun validateIsNumber(element: JsonElement): Number {
  require(
    element.isJsonPrimitive() &&
      (element.getAsJsonPrimitive().isNumber() || element.getAsJsonPrimitive().isString())
  ) {
    "Expected Integer represented as a Number or as a String; the actual element is: $element"
  }
  return element.getAsJsonPrimitive().getAsNumber()
}

/** Verifies JsonElement is Boolean. If yes, returns the value. */
private fun validateIsBoolean(element: JsonElement): Boolean {
  require(element.isJsonPrimitive() && element.getAsJsonPrimitive().isBoolean()) {
    "Expected Boolean; the actual element is: $element"
  }
  return element.getAsJsonPrimitive().getAsBoolean()
}

/** Verifies JsonElement is Double. If yes, returns the value. */
private fun validateIsDouble(element: JsonElement): Double {
  require(element.isJsonPrimitive() && element.getAsJsonPrimitive().isNumber()) {
    "Expected Double; the actual element is: $element"
  }
  return element.getAsJsonPrimitive().getAsDouble()
}

/** Verifies JsonElement is String. If yes, returns the value. */
private fun validateIsString(element: JsonElement): String {
  require(element.isJsonPrimitive() && element.getAsJsonPrimitive().isString()) {
    "Expected String; the actual element is: $element"
  }
  return element.getAsJsonPrimitive().getAsString()
}

/** Verifies JsonElement is Null. If yes, puts it into TLV. */
private fun TlvWriter.validateIsNullAndPut(element: JsonElement, tag: Tag) {
  require(element.isJsonNull()) { "Expected Null; the actual element is: $element" }
  putNull(tag)
}

/** Verifies JsonElement is JsonObject. If yes, converts it into TLV Structure. */
private fun TlvWriter.validateIsJsonObjectAndConvert(element: JsonElement, tag: Tag) {
  require(element.isJsonObject()) { "Expected JsonObject; the actual element is: $element" }
  startStructure(tag).fromJson(element.getAsJsonObject()).endStructure()
}
