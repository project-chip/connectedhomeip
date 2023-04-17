#pragma once

#include <string>
#include <lib/support/Span.h>

namespace chip {
namespace Value {

template<class SRC, class DEST>
SRC & Assign(SRC &dest, const DEST &src) {
    return dest=src;
}

template<>
std::string& Assign(std::string &dest, const chip::CharSpan &src) {
   dest = std::string(src.begin(), src.end());
   return dest;
}

template <>
chip::CharSpan & Assign(chip::CharSpan &dest, const std::string & src) {
   dest = chip::CharSpan(src.c_str(), src.size());
   return dest;
}

} // namespace Value
} // namespace chip
