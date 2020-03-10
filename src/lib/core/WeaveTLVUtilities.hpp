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
 *      This file specifies types and utility interfaces for managing and
 *      working with Weave TLV.
 *
 */

#ifndef WEAVETLVUTILITIES_HPP
#define WEAVETLVUTILITIES_HPP

#include <stddef.h>
#include <stdint.h>

#include <Weave/Core/WeaveError.h>
#include <Weave/Core/WeaveTLV.h>

namespace nl {

namespace Weave {

namespace TLV {

/**
 *   @namespace nl::Weave::TLV::Utilities
 *
 *   @brief
 *     This namespace includes types and utility interfaces for managing and
 *     working with Weave TLV.
 *
 */
namespace Utilities {

typedef WEAVE_ERROR (*IterateHandler)(const TLVReader &aReader, size_t aDepth, void *aContext);

extern WEAVE_ERROR Iterate(const TLVReader &aReader, IterateHandler aHandler, void *aContext);
extern WEAVE_ERROR Iterate(const TLVReader &aReader, IterateHandler aHandler, void *aContext, const bool aRecurse);

extern WEAVE_ERROR Count(const TLVReader &aReader, size_t &aCount);
extern WEAVE_ERROR Count(const TLVReader &aReader, size_t &aCount, const bool aRecurse);

extern WEAVE_ERROR Find(const TLVReader &aReader, const uint64_t &aTag, TLVReader &aResult);
extern WEAVE_ERROR Find(const TLVReader &aReader, const uint64_t &aTag, TLVReader &aResult, const bool aRecurse);

extern WEAVE_ERROR Find(const TLVReader &aReader, IterateHandler aHandler, void *aContext, TLVReader &aResult);
extern WEAVE_ERROR Find(const TLVReader &aReader, IterateHandler aHandler, void *aContext, TLVReader &aResult, const bool aRecurse);
} // namespace Utilities

} // namespace TLV

} // namespace Weave

} // namespace nl

#endif // WEAVETLVUTILITIES_HPP
