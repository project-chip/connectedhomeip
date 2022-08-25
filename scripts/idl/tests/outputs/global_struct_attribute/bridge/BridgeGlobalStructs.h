#pragma once

#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/attribute-id.h>
#include <app/data-model/Nullable.h>

namespace clusters {


struct LabelStruct
{
  CHIP_ERROR Decode(chip::TLV::TLVReader & reader)
  {
    chip::app::Clusters::detail::Structs::LabelStruct::DecodableType t;
    CHIP_ERROR err = t.Decode(reader);
    if(err == CHIP_NO_ERROR) {
      label = t.label;
      value = t.value;
    }
    return err;
  }

  CHIP_ERROR Encode(chip::TLV::TLVWriter & writer, chip::TLV::Tag tag) const
  {
    chip::app::Clusters::detail::Structs::LabelStruct::Type t;
    t.label = label;
    t.value = value;
    return t.Encode(writer, tag);
  }
  FixedOctetString<16, ZCL_CHAR_STRING_ATTRIBUTE_TYPE> label;
  FixedOctetString<16, ZCL_CHAR_STRING_ATTRIBUTE_TYPE> value;
};

}