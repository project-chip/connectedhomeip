/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file contains definitions for standard CHIP Attributes
 *
 */

#ifndef CHIPSTANDARDATTRIBUTES_H_
#define CHIPSTANDARDATTRIBUTES_H_

#include <lib/core/CHIPBaseAttribute.h>

namespace chip {
namespace DataModel {
#if 0
template <typename AttrId, typename CHIPTypeName, typename TypeName>
class CHIPSimpleAttribute : public CHIPBaseAttribute
{
 public:
 CHIPSimpleAttribute() : CHIPBaseAttribute(AttrId, CHIPTypeName) {}

 int Set(const TypeName newValue)
 {
     return CHIPBaseAttribute::Set(newValue);
 }
 bool Get()
 {
     TypeName value;
     CHIPBaseAttribute::Get(value);
     return value;
 }
};

template <typename AttrId, typename CHIPTypeName, typename TypeName, typename MinRange, typename MaxRange>
class CHIPRangeAttribute : public CHIPBaseAttribute
{
 public:
 CHIPSimpleAttribute() : CHIPBaseAttribute(AttrId, CHIPTypeName, MinRange, MaxRange) {}

 int Set(const TypeName newValue)
 {
     return CHIPBaseAttribute::Set(newValue);
 }
 bool Get()
 {
     TypeName value;
     CHIPBaseAttribute::Get(value);
     return value;
 }
};

/* This will mostly be auto-generated */
using CHIPAttributeOnOff = class CHIPSimpleAttribute<0x0000, kCHIPValueType_Bool, bool>;
using CHIPAttributeGlobalSceneControl = class CHIPSimpleAttribute<0x4000, kCHIPValueType_Bool, bool>;
using CHIPAttributeOnTime = class CHIPSimpleAttribute<0x4001, kCHIPValueType_UInt16, uint16_t>;
using CHIPAttributeOffWaitTime = class CHIPSimpleAttribute<0x4002, kCHIPValueType_UInt16, uint16_t>;
#endif

/* TODO: It would be nice to use a pool allocator here once we have one */
/* Base Cluster */
static inline CHIPBaseAttribute *CHIPAttributeZCLVersionNew(uint8_t ZCLVersion)
{
    return new CHIPBaseAttribute(0x0000, CHIPValueUInt8(ZCLVersion));
}

static inline CHIPBaseAttribute *CHIPAttributeApplicationVersionNew(uint8_t applicationVersion)
{
    return new CHIPBaseAttribute(0x0001, CHIPValueUInt8(applicationVersion));
}

static inline CHIPBaseAttribute *CHIPAttributeStackVersionNew(uint8_t stackVersion)
{
    return new CHIPBaseAttribute(0x0002, CHIPValueUInt8(stackVersion));
}

static inline CHIPBaseAttribute *CHIPAttributeHWVersionNew(uint8_t HWVersion)
{
    return new CHIPBaseAttribute(0x0003, CHIPValueUInt8(HWVersion));
}


/* On/Off Cluster */
static inline CHIPBaseAttribute *CHIPAttributeOnOffNew(void)
{
    return new CHIPBaseAttribute(0x0000, kCHIPValueType_Bool);
}

static inline CHIPBaseAttribute *CHIPAttributeGlobalSceneControlNew(void)
{
    return new CHIPBaseAttribute(0x4000, kCHIPValueType_Bool);
}

static inline CHIPBaseAttribute *CHIPAttributeOnTimeNew(void)
{
    return new CHIPBaseAttribute(0x4001, kCHIPValueType_UInt16);
}

static inline CHIPBaseAttribute *CHIPAttributeOffWaitTimeNew(void)
{
    return new CHIPBaseAttribute(0x4002, kCHIPValueType_UInt16);
}

} // namespace DataModel
} // namespace chip

#endif /* CHIPSTANDARDATTRIBUTES_H_ */
