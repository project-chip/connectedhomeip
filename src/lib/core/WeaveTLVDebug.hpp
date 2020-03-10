/*
 *
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
 *    All rights reserved.
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

/**
 *    @file
 *      This file defines types and interfaces for debugging and
 *      logging Weave TLV.
 *
 */

#ifndef WEAVETLVDEBUG_HPP
#define WEAVETLVDEBUG_HPP

#include <stddef.h>
#include <stdint.h>

#include <Weave/Core/WeaveError.h>
#include <Weave/Core/WeaveTLV.h>

namespace nl {

namespace Weave {

namespace TLV {

/**
 *   @namespace nl::Weave::TLV::Debug
 *
 *   @brief
 *     This namespace includes types and interfaces for debugging and
 *     logging Weave TLV.
 *
 */
namespace Debug {

typedef void (*DumpWriter)(const char *aFormat, ...);

struct DumpContext {
    DumpWriter mWriter;
    void *     mContext;
};

extern const char *DecodeType(const TLVType aType);

extern const char *DecodeTagControl(const TLVTagControl aTagControl);

extern WEAVE_ERROR DumpIterator(DumpWriter aWriter, const TLVReader &aReader);

extern WEAVE_ERROR DumpHandler(const TLVReader &aReader, size_t aDepth, void *aContext);

extern WEAVE_ERROR Dump(const TLVReader &aReader, DumpWriter aWriter);

} // namespace Debug

} // namespace TLV

} // namespace Weave

} // namespace nl

#endif // WEAVETLVDEBUG_HPP
