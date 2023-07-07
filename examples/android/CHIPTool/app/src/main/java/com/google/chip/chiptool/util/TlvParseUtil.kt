package com.google.chip.chiptool.util

import chip.tlv.AnonymousTag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter
import com.google.protobuf.ByteString

object TlvParseUtil {
  fun encode(input: Boolean): ByteArray {
    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, input)
    return tlvWriter.getEncoded();
  }

  fun encode(input: String): ByteArray {
    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, input)
    return tlvWriter.getEncoded();
  }

  fun encode(input: ULong): ByteArray {
    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, input)
    return tlvWriter.getEncoded();
  }

  fun encode(input: Long): ByteArray {
    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, input)
    return tlvWriter.getEncoded();
  }

  fun encode(input: UInt): ByteArray {
    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, input)
    return tlvWriter.getEncoded();
  }

  fun encode(input: Int): ByteArray {
    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, input)
    return tlvWriter.getEncoded();
  }

  fun encode(input: Float): ByteArray {
    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, input)
    return tlvWriter.getEncoded();
  }

  fun encode(input: Double): ByteArray {
    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, input)
    return tlvWriter.getEncoded();
  }

  fun encode(input: ByteArray): ByteArray {
    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, input)
    return tlvWriter.getEncoded();
  }

  fun decodeBoolean(tlv: ByteArray): Boolean {
    val tlvReader = TlvReader(tlv)
    return tlvReader.getBool(AnonymousTag)
  }

  fun decodeInt(tlv: ByteArray): Int {
    val tlvReader = TlvReader(tlv)
    return tlvReader.getInt(AnonymousTag)
  }

  fun decodeUInt(tlv: ByteArray): UInt {
    val tlvReader = TlvReader(tlv)
    return tlvReader.getUInt(AnonymousTag)
  }

  fun decodeLong(tlv: ByteArray): Long {
    val tlvReader = TlvReader(tlv)
    return tlvReader.getLong(AnonymousTag)
  }

  fun decodeULong(tlv: ByteArray): ULong {
    val tlvReader = TlvReader(tlv)
    return tlvReader.getULong(AnonymousTag)
  }
}
