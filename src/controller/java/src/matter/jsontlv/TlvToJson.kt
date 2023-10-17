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
import com.google.gson.JsonNull
import com.google.gson.JsonObject
import java.util.Base64
import matter.tlv.*

/**
 * Implements Matter TLV to JSON converter.
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
 * Rules for representing integers in the Json format:
 * - If the size of the integer is less or equal to 32-bits then it is represetned as a Number.
 * - If the size of the integer is larger than 32 bits then it will be represented as a String.
 *
 * @throws IllegalArgumentException if the data was invalid
 */
fun TlvReader.toJsonString(): String {
  val element = nextElement()
  require(element.value is StructureValue) {
    "The top level element must be a structure. The actual value is ${element.value}"
  }
  require(element.tag is AnonymousTag) {
    "The top level TLV Structure MUST have anonymous tag. The actual tag is ${element.tag}"
  }
  return getStructJson().toString()
}
/**
 * Encodes TLV Structure into Json Object. The TLV reader should be positioned at the start of a TLV
 * Structure (StructureValue element). After this call the TLV reader is positioned at the end of
 * the a TLV Structure (EndOfContainerValue element).
 */
private fun TlvReader.getStructJson(): JsonObject {
  var json = JsonObject()
  while (!isEndOfTlv()) {
    val element = nextElement()
    val tag = element.tag
    val value = element.value

    val key =
      when (tag) {
        is AnonymousTag -> ""
        is ContextSpecificTag -> tag.tagNumber.toString() + ":"
        is CommonProfileTag -> tag.tagNumber.toString() + ":"
        else -> throw IllegalArgumentException("Unsupported TLV tag format: $tag")
      } + getJsonValueTypeField(value)

    when (value) {
      is IntValue -> {
        if (value.value >= Int.MIN_VALUE && value.value <= Int.MAX_VALUE) {
          json.addProperty(key, value.value)
        } else {
          json.addProperty(key, value.value.toString())
        }
      }
      is UnsignedIntValue -> {
        if (value.value.toULong() <= UInt.MAX_VALUE.toULong()) {
          json.addProperty(key, value.value)
        } else {
          json.addProperty(key, value.value.toULong().toString())
        }
      }
      is Utf8StringValue -> json.addProperty(key, value.value)
      is ByteStringValue -> json.addProperty(key, Base64.getEncoder().encodeToString(value.value))
      is BooleanValue -> json.addProperty(key, value.value)
      is FloatValue -> json.addProperty(key, validateFloat(value.value))
      is DoubleValue -> json.addProperty(key, validateDouble(value.value))
      is StructureValue -> json.add(key, getStructJson())
      is ArrayValue -> {
        val (array, type) = getArrayJsonWithElementsType()
        json.add("$key-$type", array)
      }
      is ListValue ->
        throw IllegalArgumentException("Invalid TLV element: TLV List is not supported")
      is NullValue -> json.add(key, JsonNull.INSTANCE)
      is EndOfContainerValue -> return json
    }
  }
  return json
}

/**
 * Encodes TLV Array data into Json Array. The TLV reader should be positioned at the start of a TLV
 * Array (ArrayValue element). After this call the TLV reader is positioned at the end of the a TLV
 * Structure (EndOfContainerValue element). This method returns Json encoded array and a String
 * specifying types of the elements in the array.
 */
private fun TlvReader.getArrayJsonWithElementsType(): Pair<JsonArray, String> {
  var json = JsonArray()
  var lastValue: Value = ArrayValue

  while (!isEndOfTlv()) {
    val value = nextElement().value
    if (lastValue !is ArrayValue && value !is EndOfContainerValue) {
      require(value::class == lastValue::class) {
        "Invalid TLV element: all elements in array MUST be of the same type. Value ($value) is different from previous value ($lastValue)."
      }
    }

    when (value) {
      is IntValue -> {
        if (value.value >= Int.MIN_VALUE && value.value <= Int.MAX_VALUE) {
          json.add(value.value)
        } else {
          json.add(value.value.toString())
        }
      }
      is UnsignedIntValue -> {
        if (value.value.toULong() <= UInt.MAX_VALUE) {
          json.add(value.value)
        } else {
          json.add(value.value.toULong().toString())
        }
      }
      is Utf8StringValue -> json.add(value.value)
      is ByteStringValue -> json.add(Base64.getEncoder().encodeToString(value.value))
      is BooleanValue -> json.add(value.value)
      is FloatValue -> json.add(validateFloat(value.value))
      is DoubleValue -> json.add(validateDouble(value.value))
      is StructureValue -> json.add(getStructJson())
      is ArrayValue ->
        throw IllegalArgumentException(
          "Invalid TLV element: multi-dimensional TLV Array not supported"
        )
      is ListValue -> throw IllegalArgumentException("Invalid TLV Element: TLV List not supported")
      is NullValue -> json.add(JsonNull.INSTANCE)
      is EndOfContainerValue -> {
        var subType = getJsonValueTypeField(lastValue)
        if (subType == JSON_VALUE_TYPE_ARRAY) {
          subType = JSON_VALUE_TYPE_EMPTY
        }
        return Pair(json, subType)
      }
    }

    lastValue = value
  }

  throw IllegalArgumentException(
    "Invalid TLV structure: TLV Array with last value ($lastValue) is not closed"
  )
}

/** Returns type string that should be encoded in the Json key string for the specified value. */
private fun getJsonValueTypeField(value: Value): String {
  return when (value) {
    is IntValue -> JSON_VALUE_TYPE_INT
    is UnsignedIntValue -> JSON_VALUE_TYPE_UINT
    is BooleanValue -> JSON_VALUE_TYPE_BOOL
    is FloatValue -> JSON_VALUE_TYPE_FLOAT
    is DoubleValue -> JSON_VALUE_TYPE_DOUBLE
    is ByteStringValue -> JSON_VALUE_TYPE_BYTES
    is Utf8StringValue -> JSON_VALUE_TYPE_STRING
    is NullValue -> JSON_VALUE_TYPE_NULL
    is StructureValue -> JSON_VALUE_TYPE_STRUCT
    is ArrayValue -> JSON_VALUE_TYPE_ARRAY
    else -> JSON_VALUE_TYPE_EMPTY
  }
}

/** Verifies that Float value is valid supported value. */
private fun validateFloat(value: Float): Float {
  require(value != Float.NEGATIVE_INFINITY && value != Float.POSITIVE_INFINITY) {
    "Unsupported Float Infinity value"
  }
  return value
}

/** Verifies that Double value is valid supported value. */
private fun validateDouble(value: Double): Double {
  require(value != Double.NEGATIVE_INFINITY && value != Double.POSITIVE_INFINITY) {
    "Unsupported Double Infinity value"
  }
  return value
}
