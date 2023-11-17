package com.google.chip.chiptool.util

import matter.tlv.AnonymousTag
import matter.tlv.ArrayValue
import matter.tlv.NullValue
import matter.tlv.StructureValue
import matter.tlv.Tag
import matter.tlv.TlvReader

/**
 * Encodes TLV into kotlin Object. If the TLV reader is positioned TLV Structure, Object will return
 * to json format.
 */
fun TlvReader.toAny(tag: Tag = AnonymousTag): Any? {
  val element = peekElement()
  val value = element.value
  value.toAny()?.let {
    skipElement()
    return it
  }
  return when (value) {
    is ArrayValue -> {
      buildList<Any?> {
        enterArray(tag)
        while (!isEndOfTlv() && !isEndOfContainer()) {
          add(toAny())
        }
        exitContainer()
      }
    }
    is StructureValue -> {
      buildMap<Tag, Any?> {
        enterStructure(tag)
        while (!isEndOfTlv() && !isEndOfContainer()) {
          put(getTag(), toAny(getTag()))
        }
        exitContainer()
      }
    }
    is NullValue -> {
      skipElement()
      null
    }
    else -> {
      skipElement()
      null
    }
  }
}

private fun TlvReader.getTag(): Tag {
  return peekElement().tag
}
