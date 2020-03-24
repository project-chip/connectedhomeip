/*
 *
 *    Copyright (c) 2014-2017 Nest Labs, Inc.
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
 *      This file defines Weave vendor identifiers.
 *
 *      Vendor identifiers are assigned and administered by Nest Labs.
 *
 */

#ifndef WEAVE_VENDOR_IDENTIFIERS_HPP
#define WEAVE_VENDOR_IDENTIFIERS_HPP

namespace nl {

namespace Weave {

//
// Weave Vendor Identifiers (16 bits max)
//

enum WeaveVendorId
{
    kWeaveVendor_Common                         = 0x0000,
    kWeaveVendor_NestLabs                       = 0x235A,
    kWeaveVendor_Yale                           = 0xE727,
    kWeaveVendor_Google                         = 0xE100,

    kWeaveVendor_NotSpecified                   = 0xFFFF
};

}; // namespace Weave

}; // namespace nl

#endif // WEAVE_VENDOR_IDENTIFIERS_HPP
