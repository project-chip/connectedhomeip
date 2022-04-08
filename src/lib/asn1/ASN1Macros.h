/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      This file defines convenience macros for dealing with Abstract
 *      Syntax Notation One (ASN.1) encoded data.
 *
 */

#pragma once

#include <lib/support/CodeUtils.h>

// Local variable names used by utility macros.

#define ASN1_READER reader
#define ASN1_ERR err

// Utility Macros for parsing ASN1

#define ASN1_VERIFY_TAG(CLASS, TAG)                                                                                                \
    VerifyOrExit(ASN1_READER.GetClass() == (CLASS) && ASN1_READER.GetTag() == (TAG), ASN1_ERR = ASN1_ERROR_INVALID_ENCODING);

#define ASN1_PARSE_ELEMENT(CLASS, TAG)                                                                                             \
    do                                                                                                                             \
    {                                                                                                                              \
        ASN1_ERR = ASN1_READER.Next();                                                                                             \
        SuccessOrExit(ASN1_ERR);                                                                                                   \
                                                                                                                                   \
        ASN1_VERIFY_TAG(CLASS, TAG);                                                                                               \
    } while (0)

#define ASN1_PARSE_ANY                                                                                                             \
    do                                                                                                                             \
    {                                                                                                                              \
        ASN1_ERR = ASN1_READER.Next();                                                                                             \
        SuccessOrExit(ASN1_ERR);                                                                                                   \
    } while (0)

#define ASN1_ENTER_CONSTRUCTED(CLASS, TAG)                                                                                         \
    do                                                                                                                             \
    {                                                                                                                              \
        ASN1_VERIFY_TAG(CLASS, TAG);                                                                                               \
                                                                                                                                   \
        ASN1_ERR = ASN1_READER.EnterConstructedType();                                                                             \
        SuccessOrExit(ASN1_ERR);

#define ASN1_PARSE_ENTER_CONSTRUCTED(CLASS, TAG)                                                                                   \
    do                                                                                                                             \
    {                                                                                                                              \
        ASN1_ERR = ASN1_READER.Next();                                                                                             \
        SuccessOrExit(ASN1_ERR);                                                                                                   \
                                                                                                                                   \
        ASN1_VERIFY_TAG(CLASS, TAG);                                                                                               \
                                                                                                                                   \
        ASN1_ERR = ASN1_READER.EnterConstructedType();                                                                             \
        SuccessOrExit(ASN1_ERR);

#define ASN1_EXIT_CONSTRUCTED                                                                                                      \
    ASN1_ERR = ASN1_READER.Next();                                                                                                 \
    if (ASN1_ERR == CHIP_NO_ERROR)                                                                                                 \
        ASN1_ERR = ASN1_ERROR_INVALID_ENCODING;                                                                                    \
    else if (ASN1_ERR == ASN1_END)                                                                                                 \
        ASN1_ERR = CHIP_NO_ERROR;                                                                                                  \
    SuccessOrExit(ASN1_ERR);                                                                                                       \
                                                                                                                                   \
    ASN1_ERR = ASN1_READER.ExitConstructedType();                                                                                  \
    SuccessOrExit(ASN1_ERR);                                                                                                       \
    }                                                                                                                              \
    while (0)

#define ASN1_SKIP_AND_EXIT_CONSTRUCTED                                                                                             \
    ASN1_ERR = ASN1_READER.ExitConstructedType();                                                                                  \
    SuccessOrExit(ASN1_ERR);                                                                                                       \
    }                                                                                                                              \
    while (0)

#define ASN1_PARSE_ENTER_SEQUENCE ASN1_PARSE_ENTER_CONSTRUCTED(kASN1TagClass_Universal, kASN1UniversalTag_Sequence)

#define ASN1_ENTER_SEQUENCE ASN1_ENTER_CONSTRUCTED(kASN1TagClass_Universal, kASN1UniversalTag_Sequence)

#define ASN1_EXIT_SEQUENCE ASN1_EXIT_CONSTRUCTED

#define ASN1_SKIP_AND_EXIT_SEQUENCE ASN1_SKIP_AND_EXIT_CONSTRUCTED

#define ASN1_PARSE_ENTER_SET ASN1_PARSE_ENTER_CONSTRUCTED(kASN1TagClass_Universal, kASN1UniversalTag_Set)

#define ASN1_ENTER_SET ASN1_ENTER_CONSTRUCTED(kASN1TagClass_Universal, kASN1UniversalTag_Set)

#define ASN1_EXIT_SET ASN1_EXIT_CONSTRUCTED

#define ASN1_SKIP_AND_EXIT_SET ASN1_SKIP_AND_EXIT_CONSTRUCTED

#define ASN1_ENTER_ENCAPSULATED(CLASS, TAG)                                                                                        \
    do                                                                                                                             \
    {                                                                                                                              \
        ASN1_VERIFY_TAG(CLASS, TAG);                                                                                               \
                                                                                                                                   \
        ASN1_ERR = ASN1_READER.EnterEncapsulatedType();                                                                            \
        SuccessOrExit(ASN1_ERR);

#define ASN1_PARSE_ENTER_ENCAPSULATED(CLASS, TAG)                                                                                  \
    do                                                                                                                             \
    {                                                                                                                              \
        ASN1_ERR = ASN1_READER.Next();                                                                                             \
        SuccessOrExit(ASN1_ERR);                                                                                                   \
                                                                                                                                   \
        ASN1_VERIFY_TAG(CLASS, TAG);                                                                                               \
                                                                                                                                   \
        ASN1_ERR = ASN1_READER.EnterEncapsulatedType();                                                                            \
        SuccessOrExit(ASN1_ERR);

#define ASN1_EXIT_ENCAPSULATED                                                                                                     \
    ASN1_ERR = ASN1_READER.Next();                                                                                                 \
    if (ASN1_ERR == CHIP_NO_ERROR)                                                                                                 \
        ASN1_ERR = ASN1_ERROR_INVALID_ENCODING;                                                                                    \
    else if (ASN1_ERR == ASN1_END)                                                                                                 \
        ASN1_ERR = CHIP_NO_ERROR;                                                                                                  \
    SuccessOrExit(ASN1_ERR);                                                                                                       \
                                                                                                                                   \
    ASN1_ERR = ASN1_READER.ExitEncapsulatedType();                                                                                 \
    SuccessOrExit(ASN1_ERR);                                                                                                       \
    }                                                                                                                              \
    while (0)

#define ASN1_PARSE_INTEGER(OUT_VAL)                                                                                                \
    do                                                                                                                             \
    {                                                                                                                              \
        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_Integer);                                                    \
                                                                                                                                   \
        ASN1_ERR = ASN1_READER.GetInteger(OUT_VAL);                                                                                \
        SuccessOrExit(ASN1_ERR);                                                                                                   \
    } while (0)

#define ASN1_GET_INTEGER(OUT_VAL)                                                                                                  \
    do                                                                                                                             \
    {                                                                                                                              \
        ASN1_ERR = ASN1_READER.GetInteger(OUT_VAL);                                                                                \
        SuccessOrExit(ASN1_ERR);                                                                                                   \
    } while (0)

#define ASN1_PARSE_BOOLEAN(OUT_VAL)                                                                                                \
    do                                                                                                                             \
    {                                                                                                                              \
        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_Boolean);                                                    \
                                                                                                                                   \
        ASN1_ERR = ASN1_READER.GetBoolean(OUT_VAL);                                                                                \
        SuccessOrExit(ASN1_ERR);                                                                                                   \
    } while (0)

#define ASN1_GET_BOOLEAN(OUT_VAL)                                                                                                  \
    do                                                                                                                             \
    {                                                                                                                              \
        ASN1_ERR = ASN1_READER.GetBoolean(OUT_VAL);                                                                                \
        SuccessOrExit(ASN1_ERR);                                                                                                   \
    } while (0)

#define ASN1_PARSE_OBJECT_ID(OUT_VAL)                                                                                              \
    do                                                                                                                             \
    {                                                                                                                              \
        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_ObjectId);                                                   \
                                                                                                                                   \
        ASN1_ERR = ASN1_READER.GetObjectId(OUT_VAL);                                                                               \
        SuccessOrExit(ASN1_ERR);                                                                                                   \
    } while (0)

#define ASN1_GET_OBJECT_ID(OUT_VAL)                                                                                                \
    do                                                                                                                             \
    {                                                                                                                              \
        ASN1_ERR = ASN1_READER.GetObjectId(OUT_VAL);                                                                               \
        SuccessOrExit(ASN1_ERR);                                                                                                   \
    } while (0)

#define ASN1_PARSE_NULL ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_Null)

#define ASN1_PARSE_TIME(OUT_VAL)                                                                                                   \
    do                                                                                                                             \
    {                                                                                                                              \
        ASN1_PARSE_ANY;                                                                                                            \
                                                                                                                                   \
        if (ASN1_READER.GetClass() == kASN1TagClass_Universal && ASN1_READER.GetTag() == kASN1UniversalTag_UTCTime)                \
            ASN1_ERR = ASN1_READER.GetUTCTime(OUT_VAL);                                                                            \
        else if (ASN1_READER.GetClass() == kASN1TagClass_Universal && ASN1_READER.GetTag() == kASN1UniversalTag_GeneralizedTime)   \
            ASN1_ERR = ASN1_READER.GetGeneralizedTime(OUT_VAL);                                                                    \
        else                                                                                                                       \
            ASN1_ERR = ASN1_ERROR_INVALID_ENCODING;                                                                                \
        SuccessOrExit(ASN1_ERR);                                                                                                   \
    } while (0)

#define ASN1_PARSE_BIT_STRING(OUT_VAL)                                                                                             \
    do                                                                                                                             \
    {                                                                                                                              \
        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_BitString);                                                  \
                                                                                                                                   \
        ASN1_ERR = ASN1_READER.GetBitString(OUT_VAL);                                                                              \
        SuccessOrExit(ASN1_ERR);                                                                                                   \
    } while (0)

#define ASN1_GET_BIT_STRING(OUT_VAL)                                                                                               \
    do                                                                                                                             \
    {                                                                                                                              \
        ASN1_ERR = ASN1_READER.GetBitString(OUT_VAL);                                                                              \
        SuccessOrExit(ASN1_ERR);                                                                                                   \
    } while (0)

// Utility Macros for encoding ASN1

#define ASN1_START_CONSTRUCTED(CLASS, TAG)                                                                                         \
    do                                                                                                                             \
    {                                                                                                                              \
        ASN1_ERR = writer.StartConstructedType(CLASS, TAG);                                                                        \
        SuccessOrExit(ASN1_ERR);

#define ASN1_END_CONSTRUCTED                                                                                                       \
    ASN1_ERR = writer.EndConstructedType();                                                                                        \
    SuccessOrExit(ASN1_ERR);                                                                                                       \
    }                                                                                                                              \
    while (0)

#define ASN1_START_SEQUENCE ASN1_START_CONSTRUCTED(kASN1TagClass_Universal, kASN1UniversalTag_Sequence)

#define ASN1_END_SEQUENCE ASN1_END_CONSTRUCTED

#define ASN1_START_SET ASN1_START_CONSTRUCTED(kASN1TagClass_Universal, kASN1UniversalTag_Set)

#define ASN1_END_SET ASN1_END_CONSTRUCTED

#define ASN1_ENCODE_INTEGER(VAL)                                                                                                   \
    do                                                                                                                             \
    {                                                                                                                              \
        ASN1_ERR = writer.PutInteger(VAL);                                                                                         \
        SuccessOrExit(ASN1_ERR);                                                                                                   \
    } while (0);

#define ASN1_ENCODE_BOOLEAN(VAL)                                                                                                   \
    do                                                                                                                             \
    {                                                                                                                              \
        ASN1_ERR = writer.PutBoolean(VAL);                                                                                         \
        SuccessOrExit(ASN1_ERR);                                                                                                   \
    } while (0);

#define ASN1_ENCODE_BIT_STRING(VAL)                                                                                                \
    do                                                                                                                             \
    {                                                                                                                              \
        ASN1_ERR = writer.PutBitString(VAL);                                                                                       \
        SuccessOrExit(ASN1_ERR);                                                                                                   \
    } while (0);

#define ASN1_ENCODE_OBJECT_ID(OID)                                                                                                 \
    do                                                                                                                             \
    {                                                                                                                              \
        ASN1_ERR = writer.PutObjectId(OID);                                                                                        \
        SuccessOrExit(ASN1_ERR);                                                                                                   \
    } while (0);

#define ASN1_ENCODE_STRING(TAG, VAL, LEN)                                                                                          \
    do                                                                                                                             \
    {                                                                                                                              \
        ASN1_ERR = writer.PutString(TAG, VAL, LEN);                                                                                \
        SuccessOrExit(ASN1_ERR);                                                                                                   \
    } while (0);

#define ASN1_ENCODE_OCTET_STRING(VAL, LEN)                                                                                         \
    do                                                                                                                             \
    {                                                                                                                              \
        ASN1_ERR = writer.PutOctetString(VAL, LEN);                                                                                \
        SuccessOrExit(ASN1_ERR);                                                                                                   \
    } while (0);

#define ASN1_ENCODE_TIME(VAL)                                                                                                      \
    do                                                                                                                             \
    {                                                                                                                              \
        ASN1_ERR = writer.PutTime(VAL);                                                                                            \
        SuccessOrExit(ASN1_ERR);                                                                                                   \
    } while (0);

#define ASN1_ENCODE_NULL                                                                                                           \
    do                                                                                                                             \
    {                                                                                                                              \
        ASN1_ERR = writer.PutNull();                                                                                               \
        SuccessOrExit(ASN1_ERR);                                                                                                   \
    } while (0);

#define ASN1_START_ENCAPSULATED(CLASS, TAG, BIT_STRING_ENCODING)                                                                   \
    do                                                                                                                             \
    {                                                                                                                              \
        ASN1_ERR = writer.StartEncapsulatedType(CLASS, TAG, BIT_STRING_ENCODING);                                                  \
        SuccessOrExit(ASN1_ERR);

#define ASN1_START_OCTET_STRING_ENCAPSULATED ASN1_START_ENCAPSULATED(kASN1TagClass_Universal, kASN1UniversalTag_OctetString, false)

#define ASN1_START_BIT_STRING_ENCAPSULATED ASN1_START_ENCAPSULATED(kASN1TagClass_Universal, kASN1UniversalTag_BitString, true)

#define ASN1_END_ENCAPSULATED                                                                                                      \
    ASN1_ERR = writer.EndEncapsulatedType();                                                                                       \
    SuccessOrExit(ASN1_ERR);                                                                                                       \
    }                                                                                                                              \
    while (0)
