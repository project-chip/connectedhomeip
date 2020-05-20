/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Silicon Laboratories Inc. www.silabs.com
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
 *      This file provides the utility layer for the struct/spec system.
 *
 */

#ifdef CHIP_TEST
#include "utest.h"
#endif

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_STACK
#ifdef CHIP_AF_API_DEBUG_PRINT
#include CHIP_AF_API_DEBUG_PRINT
#endif
#include CHIP_AF_API_ZCL_CORE

#include <assert.h>

#include "chip-zcl-struct.h"
#include "chip-zcl.h"
#include "gen-attribute-type.h"
#include "gen-callbacks.h"
#include "gen-endpoint-config.h"
#include "zcl-data-model.h"
// Functions for extracting information about structs and fields.

uint16_t chipZclLowHighInt16u(const uint8_t * contents)
{
    return HIGH_LOW_TO_INT(contents[1], contents[0]);
}

void chipZclResetFieldData(ChipZclStructData * structData)
{
    structData->fieldIndex = 0;
    // Skip leading header/names.
    structData->next = structData->spec + CHIP_ZCL_STRUCT_HEADER_SIZE;
}

bool chipZclExpandStructData(const ChipZclStructSpec * spec, ChipZclStructData * structData)
{
    uint32_t objectData = spec[0];

    if (LOW_BYTE(objectData) != CHIP_ZCL_STRUCT_START_MARKER)
    {
        return false;
    }

    structData->spec       = spec;
    structData->size       = objectData >> 16;
    structData->fieldCount = HIGH_BYTE(objectData);
    chipZclResetFieldData(structData);

    return true;
}

bool chipZclFieldDataFinished(ChipZclStructData * structData)
{
    return structData->fieldIndex == structData->fieldCount;
}

void chipZclGetNextFieldData(ChipZclStructData * structData, ChipZclFieldData * fieldData)
{
    const ChipZclStructSpec * data = &structData->next[0];

    fieldData->valueType   = LOW_BYTE(data[0]);
    fieldData->isMandatory = fieldData->valueType & CHIP_ZCL_STRUCT_TYPE_MODIFIER_MANDATORY;
    fieldData->isArray     = fieldData->valueType & CHIP_ZCL_STRUCT_TYPE_MODIFIER_ARRAY;
    fieldData->isTlv       = fieldData->valueType & CHIP_ZCL_STRUCT_TYPE_MODIFIER_TLV;
    fieldData->valueType &= ~(CHIP_ZCL_STRUCT_TYPE_MODIFIER_MANDATORY | CHIP_ZCL_STRUCT_TYPE_MODIFIER_ARRAY |
                              CHIP_ZCL_STRUCT_TYPE_MODIFIER_TLV); // Clear valueType enum override bits after checking.

    fieldData->valueSize   = HIGH_BYTE(data[0]);
    fieldData->valueOffset = data[0] >> 16;

    if (fieldData->isTlv)
    {
        fieldData->tlvTag = data[1] & TLV_TAG_MASK;
        fieldData->name   = NULL;
    }
    else
    {
        fieldData->tlvTag = 0;
        fieldData->name   = (char *) data[1];
    }
    // Get ready for the next field.
    structData->fieldIndex += 1;
    structData->next += CHIP_ZCL_STRUCT_HEADER_SIZE;
}

//----------------------------------------------------------------
// Utilities for reading and writing integer fields.  These should probably
// go somewhere else.

uint32_t chipZclFetchInt32uValue(const uint8_t * valueLoc, uint16_t valueSize)
{
    switch (valueSize)
    {
    case 1:
        return *((const uint8_t *) valueLoc);
        break;
    case 2:
        return *((const uint16_t *) ((const void *) valueLoc));
        break;
    case 4:
        return *((const uint32_t *) ((const void *) valueLoc));
        break;
    default:
        assert(false);
        return 0;
    }
}

int32_t chipZclFetchInt32sValue(const uint8_t * valueLoc, uint16_t valueSize)
{
    switch (valueSize)
    {
    case 1:
        return *((const int8_t *) valueLoc);
    case 2:
        return *((const int16_t *) ((const void *) valueLoc));
    case 4:
        return *((const int32_t *) ((const void *) valueLoc));
    default:
        assert(false);
        return 0;
    }
}

void chipZclStoreInt32sValue(uint8_t * valueLoc, int32_t value, uint8_t valueSize)
{
    switch (valueSize)
    {
    case 1:
        *((int8_t *) valueLoc) = value;
        break;
    case 2:
        *((int16_t *) ((void *) valueLoc)) = value;
        break;
    case 4:
        *((int32_t *) ((void *) valueLoc)) = value;
        break;
    default:
        assert(false);
    }
}

void chipZclStoreInt32uValue(uint8_t * valueLoc, uint32_t value, uint8_t valueSize)
{
    switch (valueSize)
    {
    case 1:
        *((uint8_t *) valueLoc) = value;
        break;
    case 2:
        *((uint16_t *) ((void *) valueLoc)) = value;
        break;
    case 4:
        *((uint32_t *) ((void *) valueLoc)) = value;
        break;
    default:
        assert(false);
    }
}
