#pragma once

#include <app-common/zap-generated/attribute-id.h>
#include <app/data-model/Nullable.h>

#include <string>
#include <vector>

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
  std::string label;
  std::string value;
};

}
