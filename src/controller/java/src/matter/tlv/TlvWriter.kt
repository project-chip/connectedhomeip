/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    Copyright (c) 2019-2023 Google LLC.
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

package matter.tlv

import java.io.ByteArrayOutputStream

/**
 * Implements Matter TLV writer that supports all values and tags as defined in the Spec.
 *
 * @param bytes the bytes to interpret
 */
class TlvWriter(initialCapacity: Int = 32) {
  private val bytes = ByteArrayOutputStream(/* size= */ initialCapacity)
  private var containerDepth: Int = 0
  private var containerType = Array<Type>(4) { NullType() }

  /**
   * Writes the next element to the TLV.
   *
   * @throws TlvEncodingException if the data was invalid
   */
  private fun put(element: Element): TlvWriter {
    val value = element.value
    val tag = element.tag
    val type = value.toType()
    val encodedType = type.encode()

    if (containerDepth == 0) {
      require(tag !is ContextSpecificTag) {
        "Invalid use of context tag at index ${bytes.size()}: can only be used within a " +
          "structure or a list"
      }
    } else if (containerType[containerDepth - 1] is ArrayType) {
      require(tag is AnonymousTag) {
        "Invalid element tag at index ${bytes.size()}: elements of an array SHALL be anonymous"
      }
    } else if (containerType[containerDepth - 1] is StructureType && type !is EndOfContainerType) {
      require(tag !is AnonymousTag) {
        "Invalid element tag at index ${bytes.size()}: elements of a structure cannot be anonymous"
      }
    }

    if (tag is ContextSpecificTag) {
      require(tag.tagNumber.toUInt() <= UByte.MAX_VALUE) {
        "Invalid context specific tag " + "value ${tag.tagNumber} at index ${bytes.size()}"
      }
    }

    // Update depth if the element is an end of container
    if (value is EndOfContainerValue) {
      require(containerDepth > 0) {
        "Cannot close container at index ${bytes.size()}, which is not in the open container."
      }
      containerDepth--
    }

    // Encode control byte and tag
    val encodedControlAndTag: ByteArray =
      runCatching { Tag.encode(encodedType, tag) }
        .onFailure {
          throw TlvEncodingException(
            "Type error at ${bytes.size()} for ${encodedType.toBinary()}",
            it
          )
        }
        .getOrThrow()
    bytes.write(encodedControlAndTag)

    // Encode length and the value
    bytes.write(value.encode())

    // Update depth if the element is a start of container
    if (value is StructureValue || value is ArrayValue || value is ListValue) {
      if (containerType.size == containerDepth) {
        containerType = containerType.plus(type)
      } else {
        containerType[containerDepth] = type
      }
      containerDepth++
    }

    return this
  }

  /**
   * Writes the next Long element to the TLV.
   *
   * @throws TlvEncodingException if the data was invalid
   */
  fun put(tag: Tag, value: Long): TlvWriter {
    return put(Element(tag, IntValue(value)))
  }

  /**
   * Writes the next ULong element to the TLV.
   *
   * @throws TlvEncodingException if the data was invalid
   */
  fun put(tag: Tag, value: ULong): TlvWriter {
    return put(Element(tag, UnsignedIntValue(value.toLong())))
  }

  /**
   * Writes the next ULong, UInt, UShort, or UByte element to the TLV.
   *
   * This method is functionally equivalent to fun put(tag: Tag, value: ULong): TlvWriter is
   * required when called from Java, which doesn't support Unsigned integer types.
   *
   * @throws TlvEncodingException if the data was invalid
   */
  fun putUnsigned(tag: Tag, value: Number): TlvWriter {
    return put(Element(tag, UnsignedIntValue(value.toLong())))
  }

  /**
   * Writes the next Int element to the TLV.
   *
   * @throws TlvEncodingException if the data was invalid
   */
  fun put(tag: Tag, value: Int): TlvWriter {
    return put(Element(tag, IntValue(value.toLong())))
  }

  /**
   * Writes the next UInt element to the TLV.
   *
   * @throws TlvEncodingException if the data was invalid
   */
  fun put(tag: Tag, value: UInt): TlvWriter {
    return put(Element(tag, UnsignedIntValue(value.toLong())))
  }

  /**
   * Writes the next Short element to the TLV.
   *
   * @throws TlvEncodingException if the data was invalid
   */
  fun put(tag: Tag, value: Short): TlvWriter {
    return put(Element(tag, IntValue(value.toLong())))
  }

  /**
   * Writes the next UShort element to the TLV.
   *
   * @throws TlvEncodingException if the data was invalid
   */
  fun put(tag: Tag, value: UShort): TlvWriter {
    return put(Element(tag, UnsignedIntValue(value.toLong())))
  }

  /**
   * Writes the next Byte element to the TLV.
   *
   * @throws TlvEncodingException if the data was invalid
   */
  fun put(tag: Tag, value: Byte): TlvWriter {
    return put(Element(tag, IntValue(value.toLong())))
  }

  /**
   * Writes the next UByte element to the TLV.
   *
   * @throws TlvEncodingException if the data was invalid
   */
  fun put(tag: Tag, value: UByte): TlvWriter {
    return put(Element(tag, UnsignedIntValue(value.toLong())))
  }

  /**
   * Writes the next Boolean element to the TLV.
   *
   * @throws TlvEncodingException if the data was invalid
   */
  fun put(tag: Tag, value: Boolean): TlvWriter {
    return put(Element(tag, BooleanValue(value)))
  }

  /**
   * Writes the next Float element to the TLV.
   *
   * @throws TlvEncodingException if the data was invalid
   */
  fun put(tag: Tag, value: Float): TlvWriter {
    return put(Element(tag, FloatValue(value)))
  }

  /**
   * Writes the next Double element to the TLV.
   *
   * @throws TlvEncodingException if the data was invalid
   */
  fun put(tag: Tag, value: Double): TlvWriter {
    return put(Element(tag, DoubleValue(value)))
  }

  /**
   * Writes the next UTF8 String element to the TLV.
   *
   * @throws TlvEncodingException if the data was invalid
   */
  fun put(tag: Tag, value: String): TlvWriter {
    return put(Element(tag, Utf8StringValue(value)))
  }

  /**
   * Writes the next Octet String element to the TLV.
   *
   * @throws TlvEncodingException if the data was invalid
   */
  fun put(tag: Tag, value: ByteArray): TlvWriter {
    return put(Element(tag, ByteStringValue(value)))
  }

  /**
   * Writes an array of Signed Long elements to the TLV.
   *
   * Note that this method can only be used when all elements of an array are of Signed Long type.
   *
   * @throws TlvEncodingException if the data was invalid
   */
  fun putSignedLongArray(tag: Tag, array: LongArray): TlvWriter {
    startArray(tag)
    array.forEach { put(AnonymousTag, it) }
    return endArray()
  }

  /**
   * Writes an array of Unsigned Long elements to the TLV.
   *
   * Note that this method can only be used when all elements of an array are of ULong type.
   *
   * @throws TlvEncodingException if the data was invalid
   */
  fun putUnsignedLongArray(tag: Tag, array: LongArray): TlvWriter {
    startArray(tag)
    array.forEach { put(AnonymousTag, it.toULong()) }
    return endArray()
  }

  /**
   * Writes an array of Octet String elements to the TLV.
   *
   * Note that this method can only be used when all elements of an array are of Octet String type.
   *
   * @throws TlvEncodingException if the data was invalid
   */
  fun putByteStringArray(tag: Tag, array: List<ByteArray>): TlvWriter {
    startArray(tag)
    array.forEach { put(AnonymousTag, it) }
    return endArray()
  }

  /**
   * Writes the next Null element to the TLV.
   *
   * @throws TlvEncodingException if the tag was invalid
   */
  fun putNull(tag: Tag): TlvWriter {
    return put(Element(tag, NullValue))
  }

  /**
   * Writes the next element identifying the start of a Structure to the TLV.
   *
   * @throws TlvEncodingException if the tag was invalid
   */
  fun startStructure(tag: Tag): TlvWriter {
    return put(Element(tag, StructureValue))
  }

  /**
   * Writes the next element identifying the start of an Array to the TLV.
   *
   * @throws TlvEncodingException if the tag was invalid
   */
  fun startArray(tag: Tag): TlvWriter {
    return put(Element(tag, ArrayValue))
  }

  /**
   * Writes the next element identifying the start of a List to the TLV.
   *
   * @throws TlvEncodingException if the tag was invalid
   */
  fun startList(tag: Tag): TlvWriter {
    return put(Element(tag, ListValue))
  }

  /** Writes the End of Container element to the TLV. */
  fun endStructure(): TlvWriter {
    require((containerDepth > 0) && (containerType[containerDepth - 1] is StructureType)) {
      "Error closing structure at index ${bytes.size()} as currently opened container is not " +
        "a structure"
    }
    return put(Element(AnonymousTag, EndOfContainerValue))
  }

  /** Writes the End of Container element to the TLV. */
  fun endArray(): TlvWriter {
    require(containerDepth > 0 && containerType[containerDepth - 1] is ArrayType) {
      "Error closing array at index ${bytes.size()} as currently opened container is not an array"
    }
    return put(Element(AnonymousTag, EndOfContainerValue))
  }

  /** Writes the End of Container element to the TLV. */
  fun endList(): TlvWriter {
    require(containerDepth > 0 && containerType[containerDepth - 1] is ListType) {
      "Error closing list at index ${bytes.size()} as currently opened container is not a list"
    }
    return put(Element(AnonymousTag, EndOfContainerValue))
  }

  /**
   * Copies a TLV element from a reader object into the writer.
   *
   * This method encodes a new TLV element whose type, tag and value are taken from a TlvReader
   * object. When the method is called, the supplied reader object is expected to be positioned on
   * the source TLV element. The newly encoded element will have the same type, tag and contents as
   * the input container. If the supplied element is a TLV container (structure, array or list), the
   * entire contents of the container will be copied.
   *
   * @param reader a TlvReader object positioned at a Tlv element whose tag, type and value should
   *   be copied. If this method is executed successfully, the reader will be positioned at the end
   *   of the element that was copied.
   */
  fun copyElement(reader: TlvReader): TlvWriter {
    return copyElement(reader.peekElement().tag, reader)
  }

  /**
   * Copies a TLV element from a reader object into the writer.
   *
   * This method encodes a new TLV element whose type and value are taken from a TLVReader object.
   * When the method is called, the supplied reader object is expected to be positioned on the
   * source TLV element. The newly encoded element will have the same type and contents as the input
   * container, however the tag will be set to the specified argument. If the supplied element is a
   * TLV container (structure, array or list), the entire contents of the container will be copied.
   *
   * @param tag the TLV tag to be encoded with the element.
   * @param reader a TlvReader object positioned at a Tlv element whose type and value should be
   *   copied. If this method is executed successfully, the reader will be positioned at the end of
   *   the element that was copied.
   */
  fun copyElement(tag: Tag, reader: TlvReader): TlvWriter {
    var depth = 0
    do {
      val element = reader.nextElement()
      val value = element.value

      when (depth) {
        0 -> {
          require(value !is EndOfContainerValue) {
            "The TlvReader is positioned at invalid element: EndOfContainer"
          }
          put(Element(tag, value))
        }
        else -> put(element)
      }

      if (value is EndOfContainerValue) {
        depth--
      } else if (value is StructureValue || value is ArrayValue || value is ListValue) {
        depth++
      }
    } while (depth > 0)
    return this
  }

  /** Returns the total number of bytes written since the writer was initialized. */
  fun getLengthWritten(): Int {
    return bytes.size()
  }

  /** Verifies that all open containers are closed. */
  fun validateTlv(): TlvWriter {
    if (containerDepth > 0) {
      throw TlvEncodingException("Invalid Tlv data: $containerDepth containers are not closed")
    }
    return this
  }

  /** Returns the TLV encoded data written since the writer was initialized. */
  fun getEncoded(): ByteArray {
    return bytes.toByteArray()
  }

  /** Resets the writer state to empty byte array. */
  fun reset() {
    bytes.reset()
    containerDepth = 0
    containerType = Array<Type>(4) { NullType() }
  }
}

/** Exception thrown if there was an issue encoding the TLV data. */
class TlvEncodingException internal constructor(msg: String, cause: Throwable? = null) :
  RuntimeException(msg, cause)
