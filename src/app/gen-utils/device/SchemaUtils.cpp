/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *      This file defines Base class for a CHIP IM Command
 *
 */

#include "SchemaTypes.h"
#include "core/CHIPError.h"
#include "core/CHIPTLVTags.h"
#include "core/CHIPTLVTypes.h"
#include <core/CHIPTLVDebug.hpp>

struct Test {
};

static_assert(sizeof(chip::Span<uint8_t>) == sizeof(chip::Span<Test>), "Chip::Span cannot be type erasured");

#pragma GCC diagnostic ignored "-Wstack-usage="

namespace chip {
namespace app {

CHIP_ERROR EncodeSchemaElement(chip::Span<const CompactFieldDescriptor> pDescriptor, void *buf, uint64_t tag, TLV::TLVWriter &writer, bool inArray)
{
    CHIP_ERROR err;
    TLV::TLVType outerContainerType;

    if (!inArray) {
        err = writer.StartContainer(tag, TLV::kTLVType_Structure, outerContainerType);
        SuccessOrExit(err);
    }

    for (auto schemaIter = pDescriptor.data(); schemaIter != (pDescriptor.data() + pDescriptor.size()); schemaIter++) {
        uint64_t fieldTag = inArray ? TLV::AnonymousTag : TLV::ContextTag((uint8_t)schemaIter->FieldId);

        if (schemaIter->FieldType.Has(Type::TYPE_ARRAY)) {
            TLV::TLVType outerContainerType2;
            chip::ByteSpan &p = *(reinterpret_cast<chip::ByteSpan *>((uintptr_t)(buf) + schemaIter->Offset));
            CompactFieldDescriptor tmpDescriptor = *schemaIter;
            chip::Span<const CompactFieldDescriptor> tmpDescriptorList = {&tmpDescriptor, 1};

            if (p.data() && p.size()) {
                tmpDescriptor.FieldType.Clear(Type::TYPE_ARRAY);
                tmpDescriptor.Offset = 0;

                err = writer.StartContainer(TLV::ContextTag((uint8_t)(schemaIter->FieldId)), TLV::kTLVType_Array, outerContainerType2);
                SuccessOrExit(err);

                for (const uint8_t *ptr = p.data(); ptr < (p.data() + p.size() * schemaIter->TypeSize); ptr += schemaIter->TypeSize) {
                    err = EncodeSchemaElement(tmpDescriptorList, (void *)ptr, 0, writer, true);
                    SuccessOrExit(err);
                }

                err = writer.EndContainer(outerContainerType2);
                SuccessOrExit(err);
            }
        }
        else if (schemaIter->FieldType.Has(Type::TYPE_STRUCT)) {
            err = EncodeSchemaElement(schemaIter->StructDef, (void *)((uintptr_t)(buf) + schemaIter->Offset), 
                                      fieldTag, writer, false);
            SuccessOrExit(err);
        }
        
        else if (schemaIter->FieldType.Has(Type::TYPE_UINT8)) {
            uint8_t *v = (uint8_t *)((uintptr_t)(buf) + schemaIter->Offset);
            err = writer.Put(fieldTag, *v);
            SuccessOrExit(err);
        }
        else if (schemaIter->FieldType.Has(Type::TYPE_UINT32)) {
            uint32_t *v = (uint32_t *)((uintptr_t)(buf) + schemaIter->Offset);
            err = writer.Put(fieldTag, *v);
            SuccessOrExit(err);
        }
        else if (schemaIter->FieldType.Has(Type::TYPE_OCTSTR)) {
            chip::ByteSpan &p = *(reinterpret_cast<chip::ByteSpan *>((uintptr_t)(buf) + schemaIter->Offset));

            if (p.data() && p.size()) {
                err = writer.PutBytes(TLV::ContextTag((uint8_t)(schemaIter->FieldId)), p.data(), (uint32_t)p.size());
                SuccessOrExit(err);
            }
        }
        else if (schemaIter->FieldType.Has(Type::TYPE_STRING)) {
            chip::Span<char> &p = *(reinterpret_cast<chip::Span<char>*>((uintptr_t)(buf) + schemaIter->Offset));

            if (p.data() && p.size()) {
                err = writer.PutString(TLV::ContextTag((uint8_t)(schemaIter->FieldId)), p.data(), (uint32_t)p.size());
                SuccessOrExit(err);
            }
        }
    }

    if (!inArray) {
        err = writer.EndContainer(outerContainerType);
        SuccessOrExit(err);
    }

exit:
    return err;
}

CHIP_ERROR DecodeSchemaElement(chip::Span<const CompactFieldDescriptor> pDescriptor, void *buf, TLV::TLVReader &reader, bool inArray)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVType outerContainerType;

    if (!inArray) {
        err = reader.EnterContainer(outerContainerType);
        SuccessOrExit(err);
    }

    while ((err = reader.Next()) == CHIP_NO_ERROR) {
        uint64_t tag = reader.GetTag();

        if (!inArray) {
            VerifyOrExit(TLV::IsContextTag(tag), err = CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);
        }

        if (!buf) {
            return CHIP_ERROR_NULL_BUF_ARG;
        }

        for (auto schemaIter = pDescriptor.data(); schemaIter != (pDescriptor.data() + pDescriptor.size()); schemaIter++) {
            if (inArray || (!inArray && (schemaIter->FieldId == TLV::TagNumFromTag(tag)))) {
                if (schemaIter->FieldType.Has(Type::TYPE_ARRAY)) {
                    TLV::TLVType outerContainerType2;
                    chip::ByteSpan &p = *(reinterpret_cast<chip::ByteSpan *>((uintptr_t)(buf) + schemaIter->Offset));
                    CompactFieldDescriptor tmpDescriptor = *schemaIter;
                    chip::Span<const CompactFieldDescriptor> tmpDescriptorList = {&tmpDescriptor, 1};

                    tmpDescriptor.FieldType.Clear(Type::TYPE_ARRAY);
                    tmpDescriptor.Offset = 0;

                    err = reader.EnterContainer(outerContainerType2);
                    SuccessOrExit(err);

                    {
                        const uint8_t *ptr = p.data();
                        int size = (int)p.size();

                        VerifyOrExit(size, err = CHIP_ERROR_NULL_BUF_ARG);

                        while ((err = DecodeSchemaElement(tmpDescriptorList, (void *)ptr, reader, true) == CHIP_NO_ERROR)) {
                            size--;

                            if (size == 0) {
                                ptr = nullptr;
                            }
                            else {
                                ptr += schemaIter->TypeSize;
                            }
                        }

                    }

                    if (err == CHIP_END_OF_TLV) {
                        err = CHIP_NO_ERROR;
                    }

                    SuccessOrExit(err);

                    err = reader.ExitContainer(outerContainerType2);
                    SuccessOrExit(err);
                    break;
                }
                else if (schemaIter->FieldType.Has(Type::TYPE_STRUCT)) {
                    err = DecodeSchemaElement(schemaIter->StructDef, (void *)((uintptr_t)(buf) + schemaIter->Offset), reader, false);
                    SuccessOrExit(err);
                    break;
                }
                else if (schemaIter->FieldType.Has(Type::TYPE_UINT8)) {
                    uint8_t *v = (uint8_t *)((uintptr_t)buf + schemaIter->Offset);
                    err = reader.Get(*v);
                    SuccessOrExit(err);
                    break;
                }
                else if (schemaIter->FieldType.Has(Type::TYPE_UINT32)) {
                    uint32_t *v = (uint32_t *)((uintptr_t)buf + schemaIter->Offset);
                    err = reader.Get(*v);
                    SuccessOrExit(err);
                    break;
                }
                else if (schemaIter->FieldType.Has(Type::TYPE_UINT64)) {
                    uint64_t *v = (uint64_t *)((uintptr_t)buf + schemaIter->Offset);
                    err = reader.Get(*v);
                    SuccessOrExit(err);
                    break;
                }
                else if (schemaIter->FieldType.Has(Type::TYPE_OCTSTR)) {
                    chip::ByteSpan &p = *(reinterpret_cast<chip::ByteSpan *>((uintptr_t)(buf) + schemaIter->Offset));
                    uint32_t sz = reader.GetLength();

                    VerifyOrExit(p.data() && p.size() >= sz, err = CHIP_ERROR_NULL_BUF_ARG);

                    err = reader.GetBytes((uint8_t *)p.data(), (uint32_t)p.size());
                    SuccessOrExit(err);

                    chip::ByteSpan *p1 = (reinterpret_cast<chip::ByteSpan *>((uintptr_t)(buf) + schemaIter->Offset));
                    *p1 = chip::ByteSpan(p.data(), sz);
                }
                else if (schemaIter->FieldType.Has(Type::TYPE_STRING)) {
                    chip::Span<char> &p = *(reinterpret_cast<chip::Span<char> *>((uintptr_t)(buf) + schemaIter->Offset));
                    uint32_t sz = reader.GetLength();

                    VerifyOrExit(p.data() && p.size() >= sz, err = CHIP_ERROR_NULL_BUF_ARG);

                    err = reader.GetString((char *)p.data(), (uint32_t)p.size());
                    SuccessOrExit(err);

                    chip::Span<char> *p1 = (reinterpret_cast<chip::Span<char> *>((uintptr_t)(buf) + schemaIter->Offset));
                    *p1 = chip::Span<char>(p.data(), sz);
                }
            }
        }

        if (inArray) {
            break;
        }
    }

    if (!inArray) {
        err = reader.ExitContainer(outerContainerType);
        SuccessOrExit(err);

        if (err == CHIP_END_OF_TLV) {
            err = CHIP_NO_ERROR;
        }
    }

exit:
    return err;
}


} // namespace app
} // namespace chip
