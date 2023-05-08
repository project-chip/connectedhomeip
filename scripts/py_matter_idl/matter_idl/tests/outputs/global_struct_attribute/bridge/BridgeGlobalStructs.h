#pragma once

#include <app/data-model/Nullable.h>
#include <lib/assign/ValueAssign.h>

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
      chip::Value::Assign(label,  t.label);
      chip::Value::Assign(value,  t.value);
    }
    return err;
  }

  CHIP_ERROR Encode(chip::TLV::TLVWriter & writer, chip::TLV::Tag tag) const
  {
    chip::app::Clusters::detail::Structs::LabelStruct::Type t;
      chip::Value::Assign(t.label,  label);
      chip::Value::Assign(t.value,  value);
    return t.Encode(writer, tag);
  }
  std::string label;
  std::string value;
};

}
