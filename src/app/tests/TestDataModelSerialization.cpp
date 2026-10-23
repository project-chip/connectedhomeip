/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeValueDecoder.h>
#include <app/data-model/Decode.h>
#include <app/data-model/Encode.h>
#include <lib/core/TLV.h>
#include <lib/support/CHIPMem.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <pw_unit_test/framework.h>

#include <optional>
#include <type_traits>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

struct ContextRecordingValue
{
    static constexpr bool kIsFabricScoped = false;
    uint8_t value                         = 0;
    DataModel::DecodeContext context      = DataModel::DecodeContext::kUnspecified;

    CHIP_ERROR Decode(TLV::TLVReader & reader) { return reader.Get(value); }

    CHIP_ERROR DecodeWithContext(TLV::TLVReader & reader, DataModel::DecodeContext decodeContext)
    {
        context = decodeContext;
        return reader.Get(value);
    }
};

class TestDataModelSerialization : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
    void TearDown() override { System::PacketBufferHandle buf = mStore.Release(); }

    // Helper functions
    template <typename Encodable, typename Decodable>
    void NullablesOptionalsEncodeDecodeCheck(bool encodeNulls, bool encodeValues);

    template <typename Encodable, typename Decodable>
    void NullablesOptionalsEncodeDecodeCheck();

    void SetupBuf();
    void DumpBuf();
    void SetupReader();

    template <typename Encodable>
    CHIP_ERROR EncodeStructWithoutField(TLV::Tag tag, const Encodable & value, uint8_t omittedTag,
                                        std::optional<uint8_t> replacementTag = std::nullopt);

    template <typename Encodable, typename Decodable>
    void CheckMissingRequiredFields(Span<const uint8_t> requiredTags);

    System::TLVPacketBufferBackingStore mStore;
    TLV::TLVWriter mWriter;
    TLV::TLVReader mReader;
};

using namespace TLV;

void TestDataModelSerialization::SetupBuf()
{
    System::PacketBufferHandle buf;

    buf = System::PacketBufferHandle::New(1024);
    mStore.Init(std::move(buf));

    EXPECT_SUCCESS(mWriter.Init(mStore));
    EXPECT_SUCCESS(mReader.Init(mStore));
}

void TestDataModelSerialization::DumpBuf()
{
    TLV::TLVReader reader;
    EXPECT_SUCCESS(reader.Init(mStore));

    //
    // Enable this once the TLV pretty printer has been checked in.
    //
#if defined(ENABLE_TLV_PRINT_OUT) && ENABLE_TLV_PRINT_OUT
    TLV::Debug::Print(reader);
#endif
}

void TestDataModelSerialization::SetupReader()
{

    EXPECT_SUCCESS(mReader.Init(mStore));
    EXPECT_EQ(mReader.Next(), CHIP_NO_ERROR);
}

template <typename Encodable>
CHIP_ERROR TestDataModelSerialization::EncodeStructWithoutField(TLV::Tag tag, const Encodable & value, uint8_t omittedTag,
                                                                std::optional<uint8_t> replacementTag)
{
    // Use only the isolated UnitTesting schema to exercise generated decoder behavior.
    uint8_t buffer[1024];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    if constexpr (std::is_same_v<Encodable, UnitTesting::Structs::TestFabricScoped::Type>)
    {
        ReturnErrorOnFailure(DataModel::EncodeForRead(writer, TLV::AnonymousTag(), 1, value));
    }
    else
    {
        DataModel::FabricAwareTLVWriter fabricWriter(writer, 1);
        ReturnErrorOnFailure(value.Encode(fabricWriter, TLV::AnonymousTag()));
    }
    ReturnErrorOnFailure(writer.Finalize());

    TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    ReturnErrorOnFailure(reader.Next());
    TLV::TLVType sourceContainer;
    ReturnErrorOnFailure(reader.EnterContainer(sourceContainer));
    TLV::TLVType destinationContainer;
    ReturnErrorOnFailure(mWriter.StartContainer(tag, TLV::kTLVType_Structure, destinationContainer));
    CHIP_ERROR err;
    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        if (reader.GetTag() != TLV::ContextTag(omittedTag))
        {
            ReturnErrorOnFailure(mWriter.CopyElement(reader));
        }
        else if (replacementTag.has_value())
        {
            ReturnErrorOnFailure(mWriter.CopyElement(TLV::ContextTag(*replacementTag), reader));
        }
    }
    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
    ReturnErrorOnFailure(reader.ExitContainer(sourceContainer));
    return mWriter.EndContainer(destinationContainer);
}

template <typename Encodable, typename Decodable>
void TestDataModelSerialization::CheckMissingRequiredFields(Span<const uint8_t> requiredTags)
{
    for (const auto omittedTag : requiredTags)
    {
        SetupBuf();
        ASSERT_SUCCESS(EncodeStructWithoutField(TLV::AnonymousTag(), Encodable{}, omittedTag));
        ASSERT_SUCCESS(mWriter.Finalize());
        SetupReader();
        ASSERT_FALSE(HasFailure());
        Decodable decoded;
        EXPECT_EQ(DataModel::Decode(mReader, decoded), CHIP_ERROR_MISSING_TLV_ELEMENT);
    }
}

TEST_F(TestDataModelSerialization, MissingRequiredCommandArguments)
{
    using Fields         = UnitTesting::Commands::TestAddArguments::Fields;
    const uint8_t tags[] = { to_underlying(Fields::kArg1), to_underlying(Fields::kArg2) };
    CheckMissingRequiredFields<UnitTesting::Commands::TestAddArguments::Type,
                               UnitTesting::Commands::TestAddArguments::DecodableType>(Span<const uint8_t>(tags));
}

TEST_F(TestDataModelSerialization, MissingRequiredResponseArgument)
{
    using Fields         = UnitTesting::Commands::TestAddArgumentsResponse::Fields;
    const uint8_t tags[] = { to_underlying(Fields::kReturnValue) };
    CheckMissingRequiredFields<UnitTesting::Commands::TestAddArgumentsResponse::Type,
                               UnitTesting::Commands::TestAddArgumentsResponse::DecodableType>(Span<const uint8_t>(tags));
}

TEST_F(TestDataModelSerialization, MissingRequiredStructFields)
{
    using Fields         = UnitTesting::Structs::SimpleStruct::Fields;
    const uint8_t tags[] = { to_underlying(Fields::kA), to_underlying(Fields::kB), to_underlying(Fields::kC),
                             to_underlying(Fields::kD), to_underlying(Fields::kE), to_underlying(Fields::kF),
                             to_underlying(Fields::kG), to_underlying(Fields::kH) };
    CheckMissingRequiredFields<UnitTesting::Structs::SimpleStruct::Type, UnitTesting::Structs::SimpleStruct::DecodableType>(
        Span<const uint8_t>(tags));
}

TEST_F(TestDataModelSerialization, MissingRequiredEventFields)
{
    using Fields         = UnitTesting::Events::TestEvent::Fields;
    const uint8_t tags[] = { to_underlying(Fields::kArg1), to_underlying(Fields::kArg2), to_underlying(Fields::kArg3),
                             to_underlying(Fields::kArg4), to_underlying(Fields::kArg5), to_underlying(Fields::kArg6) };
    CheckMissingRequiredFields<UnitTesting::Events::TestEvent::Type, UnitTesting::Events::TestEvent::DecodableType>(
        Span<const uint8_t>(tags));
}

TEST_F(TestDataModelSerialization, NullDoesNotMakeRequiredFieldsOptional)
{
    using Fields         = UnitTesting::Structs::NullablesAndOptionalsStruct::Fields;
    const uint8_t tags[] = { to_underlying(Fields::kNullableInt), to_underlying(Fields::kNullableString),
                             to_underlying(Fields::kNullableStruct), to_underlying(Fields::kNullableList) };
    CheckMissingRequiredFields<UnitTesting::Structs::NullablesAndOptionalsStruct::Type,
                               UnitTesting::Structs::NullablesAndOptionalsStruct::DecodableType>(Span<const uint8_t>(tags));
}

TEST_F(TestDataModelSerialization, DuplicateOrUnknownTagCannotReplaceRequiredArgument)
{
    using Fields = UnitTesting::Commands::TestAddArguments::Fields;
    for (const auto replacementTag : { to_underlying(Fields::kArg1), uint8_t{ 255 } })
    {
        SetupBuf();
        ASSERT_SUCCESS(EncodeStructWithoutField(TLV::AnonymousTag(), UnitTesting::Commands::TestAddArguments::Type{},
                                                to_underlying(Fields::kArg2), replacementTag));
        ASSERT_SUCCESS(mWriter.Finalize());
        SetupReader();
        ASSERT_FALSE(HasFailure());
        UnitTesting::Commands::TestAddArguments::DecodableType decoded;
        EXPECT_EQ(DataModel::Decode(mReader, decoded), CHIP_ERROR_MISSING_TLV_ELEMENT);
    }
}

TEST_F(TestDataModelSerialization, MissingNestedFieldReachesCommandDecoder)
{
    SetupBuf();
    TLV::TLVType container;
    ASSERT_SUCCESS(mWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container));
    ASSERT_SUCCESS(EncodeStructWithoutField(TLV::ContextTag(UnitTesting::Commands::TestStructArgumentRequest::Fields::kArg1),
                                            UnitTesting::Structs::SimpleStruct::Type{},
                                            to_underlying(UnitTesting::Structs::SimpleStruct::Fields::kA)));
    ASSERT_SUCCESS(mWriter.EndContainer(container));
    ASSERT_SUCCESS(mWriter.Finalize());
    SetupReader();
    ASSERT_FALSE(HasFailure());
    UnitTesting::Commands::TestStructArgumentRequest::DecodableType decoded;
    EXPECT_EQ(DataModel::Decode(mReader, decoded), CHIP_ERROR_MISSING_TLV_ELEMENT);
}

TEST_F(TestDataModelSerialization, MissingFieldReachesOptionalNullableDecoder)
{
    for (const auto context : { DataModel::DecodeContext::kRead, DataModel::DecodeContext::kWrite })
    {
        SetupBuf();
        ASSERT_SUCCESS(EncodeStructWithoutField(TLV::AnonymousTag(), UnitTesting::Structs::SimpleStruct::Type{},
                                                to_underlying(UnitTesting::Structs::SimpleStruct::Fields::kA)));
        ASSERT_SUCCESS(mWriter.Finalize());
        SetupReader();
        ASSERT_FALSE(HasFailure());
        Optional<DataModel::Nullable<UnitTesting::Structs::SimpleStruct::DecodableType>> decoded;
        EXPECT_EQ(DataModel::Decode(mReader, decoded, context), CHIP_ERROR_MISSING_TLV_ELEMENT);
    }
}

TEST_F(TestDataModelSerialization, MissingListElementFieldReachesIteratorStatus)
{
    for (const auto context : { DataModel::DecodeContext::kRead, DataModel::DecodeContext::kWrite })
    {
        SetupBuf();
        TLV::TLVType container;
        ASSERT_SUCCESS(mWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, container));
        UnitTesting::Structs::SimpleStruct::Type first;
        first.a = 7;
        ASSERT_SUCCESS(DataModel::Encode(mWriter, TLV::AnonymousTag(), first));
        ASSERT_SUCCESS(EncodeStructWithoutField(TLV::AnonymousTag(), UnitTesting::Structs::SimpleStruct::Type{},
                                                to_underlying(UnitTesting::Structs::SimpleStruct::Fields::kA)));
        ASSERT_SUCCESS(mWriter.EndContainer(container));
        ASSERT_SUCCESS(mWriter.Finalize());
        SetupReader();
        ASSERT_FALSE(HasFailure());
        DataModel::DecodableList<UnitTesting::Structs::SimpleStruct::DecodableType> decoded;
        ASSERT_SUCCESS(DataModel::Decode(mReader, decoded, context));
        auto iterator = decoded.begin();
        ASSERT_TRUE(iterator.Next());
        EXPECT_EQ(iterator.GetValue().a, first.a);
        EXPECT_FALSE(iterator.Next());
        EXPECT_EQ(iterator.GetStatus(), CHIP_ERROR_MISSING_TLV_ELEMENT);
    }
}

TEST_F(TestDataModelSerialization, SetReaderResetsDecodeContext)
{
    const uint8_t values[] = { 7 };
    for (const auto context : { DataModel::DecodeContext::kRead, DataModel::DecodeContext::kWrite })
    {
        SetupBuf();
        ASSERT_SUCCESS(DataModel::Encode(mWriter, TLV::AnonymousTag(), DataModel::List<const uint8_t>(values)));
        ASSERT_SUCCESS(mWriter.Finalize());
        SetupReader();
        ASSERT_FALSE(HasFailure());
        DataModel::DecodableList<ContextRecordingValue> decoded;
        ASSERT_SUCCESS(DataModel::Decode(mReader, decoded, context));
        auto initial = decoded.begin();
        ASSERT_TRUE(initial.Next());
        EXPECT_EQ(initial.GetValue().context, context);

        SetupReader();
        ASSERT_FALSE(HasFailure());
        TLV::TLVType container;
        ASSERT_SUCCESS(mReader.EnterContainer(container));
        decoded.SetReader(mReader);
        auto reused = decoded.begin();
        ASSERT_TRUE(reused.Next());
        EXPECT_EQ(reused.GetValue().value, values[0]);
        EXPECT_EQ(reused.GetValue().context, DataModel::DecodeContext::kUnspecified);
        EXPECT_FALSE(reused.Next());
        EXPECT_SUCCESS(reused.GetStatus());
    }
}

TEST_F(TestDataModelSerialization, FabricIndexIsRequiredForExplicitReadMode)
{
    UnitTesting::Structs::TestFabricScoped::Type encoded;
    encoded.SetFabricIndex(1);
    for (const auto context :
         { DataModel::DecodeContext::kRead, DataModel::DecodeContext::kWrite, DataModel::DecodeContext::kUnspecified })
    {
        SetupBuf();
        ASSERT_SUCCESS(DataModel::EncodeForWrite(mWriter, TLV::AnonymousTag(), encoded));
        ASSERT_SUCCESS(mWriter.Finalize());
        SetupReader();
        ASSERT_FALSE(HasFailure());
        UnitTesting::Structs::TestFabricScoped::DecodableType decoded;
        // Write encoding is valid without FabricIndex; a read must carry it.
        EXPECT_EQ(DataModel::Decode(mReader, decoded, context),
                  context == DataModel::DecodeContext::kRead ? CHIP_ERROR_MISSING_TLV_ELEMENT : CHIP_NO_ERROR);
    }
}

TEST_F(TestDataModelSerialization, PartialFabricRedactionIsRejected)
{
    using Fields = UnitTesting::Structs::TestFabricScoped::Fields;
    UnitTesting::Structs::TestFabricScoped::Type encoded;
    encoded.SetFabricIndex(1);
    for (const auto omittedTag :
         { to_underlying(Fields::kFabricSensitiveInt8u), to_underlying(Fields::kNullableFabricSensitiveInt8u) })
    {
        SetupBuf();
        ASSERT_SUCCESS(EncodeStructWithoutField(TLV::AnonymousTag(), encoded, omittedTag));
        ASSERT_SUCCESS(mWriter.Finalize());
        SetupReader();
        ASSERT_FALSE(HasFailure());
        UnitTesting::Structs::TestFabricScoped::DecodableType decoded;
        // Read redaction omits the complete sensitive set. Partial omission is a schema error.
        EXPECT_EQ(DataModel::Decode(mReader, decoded, DataModel::DecodeContext::kRead), CHIP_ERROR_MISSING_TLV_ELEMENT);
    }
}

TEST_F(TestDataModelSerialization, FabricReadRedactionIsRejectedForWrites)
{
    UnitTesting::Structs::TestFabricScoped::Type encoded;
    encoded.SetFabricIndex(1);
    SetupBuf();
    ASSERT_SUCCESS(DataModel::EncodeForRead(mWriter, TLV::AnonymousTag(), 2, encoded));
    ASSERT_SUCCESS(mWriter.Finalize());
    for (const auto context :
         { DataModel::DecodeContext::kRead, DataModel::DecodeContext::kWrite, DataModel::DecodeContext::kUnspecified })
    {
        SetupReader();
        ASSERT_FALSE(HasFailure());
        UnitTesting::Structs::TestFabricScoped::DecodableType decoded;
        // A complete redacted read is legal; writing it would omit required sensitive fields.
        EXPECT_EQ(DataModel::Decode(mReader, decoded, context),
                  context == DataModel::DecodeContext::kWrite ? CHIP_ERROR_MISSING_TLV_ELEMENT : CHIP_NO_ERROR);
    }
}

TEST_F(TestDataModelSerialization, FabricRedactionThroughOptionalNullableRetainsContext)
{
    UnitTesting::Structs::TestFabricScoped::Type encoded;
    encoded.SetFabricIndex(1);
    SetupBuf();
    ASSERT_SUCCESS(DataModel::EncodeForRead(mWriter, TLV::AnonymousTag(), 2, encoded));
    ASSERT_SUCCESS(mWriter.Finalize());
    for (const auto context : { DataModel::DecodeContext::kRead, DataModel::DecodeContext::kWrite })
    {
        SetupReader();
        ASSERT_FALSE(HasFailure());
        Optional<DataModel::Nullable<UnitTesting::Structs::TestFabricScoped::DecodableType>> decoded;
        EXPECT_EQ(DataModel::Decode(mReader, decoded, context),
                  context == DataModel::DecodeContext::kWrite ? CHIP_ERROR_MISSING_TLV_ELEMENT : CHIP_NO_ERROR);
    }
}

TEST_F(TestDataModelSerialization, AttributeWriteRejectsFabricReadRedaction)
{
    UnitTesting::Structs::TestFabricScoped::Type encoded;
    encoded.SetFabricIndex(1);
    SetupBuf();
    ASSERT_SUCCESS(DataModel::EncodeForRead(mWriter, TLV::AnonymousTag(), 2, encoded));
    ASSERT_SUCCESS(mWriter.Finalize());
    SetupReader();
    ASSERT_FALSE(HasFailure());
    Access::SubjectDescriptor subject = { .fabricIndex = 2 };
    AttributeValueDecoder decoder(mReader, subject);
    UnitTesting::Structs::TestFabricScoped::DecodableType decoded;
    EXPECT_EQ(decoder.Decode(decoded), CHIP_ERROR_MISSING_TLV_ELEMENT);
}

TEST_F(TestDataModelSerialization, AttributeListWriteRejectsFabricReadRedactionDuringIteration)
{
    UnitTesting::Structs::TestFabricScoped::Type encoded;
    encoded.SetFabricIndex(1);
    SetupBuf();
    ASSERT_SUCCESS(DataModel::EncodeForRead(mWriter, TLV::AnonymousTag(), 2,
                                            DataModel::List<const UnitTesting::Structs::TestFabricScoped::Type>(&encoded, 1)));
    ASSERT_SUCCESS(mWriter.Finalize());
    SetupReader();
    ASSERT_FALSE(HasFailure());
    Access::SubjectDescriptor subject = { .fabricIndex = 2 };
    AttributeValueDecoder decoder(mReader, subject);
    DataModel::DecodableList<UnitTesting::Structs::TestFabricScoped::DecodableType> decoded;
    ASSERT_SUCCESS(decoder.Decode(decoded));
    // List decoding remains lazy: the error must be checked by its consumer.
    auto iterator = decoded.begin();
    EXPECT_FALSE(iterator.Next());
    EXPECT_EQ(iterator.GetStatus(), CHIP_ERROR_MISSING_TLV_ELEMENT);
}

template <typename T>
struct TagValuePair
{
    TLV::Tag tag;
    T & value;
};

template <typename T>
TagValuePair<T> MakeTagValuePair(TLV::Tag tag, T & value)
{
    return TagValuePair<T>{ tag, value };
}

template <typename... ArgTypes>
CHIP_ERROR EncodeStruct(TLV::TLVWriter & writer, TLV::Tag tag, ArgTypes... Args)
{
    using expand_type = int[];
    TLV::TLVType type;

    ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, type));
    (void) (expand_type{ (TEMPORARY_RETURN_IGNORED DataModel::Encode(writer, Args.tag, Args.value), 0)... });
    ReturnErrorOnFailure(writer.EndContainer(type));

    return CHIP_NO_ERROR;
}

bool StringMatches(Span<const char> str1, const char * str2)
{
    if (str1.data() == nullptr || str2 == nullptr)
    {
        return false;
    }

    if (str1.size() != strlen(str2))
    {
        return false;
    }

    return (strncmp(str1.data(), str2, str1.size()) == 0);
}

TEST_F(TestDataModelSerialization, EncAndDecSimpleStruct)
{
    SetupBuf();

    //
    // Encode
    //
    {
        Clusters::UnitTesting::Structs::SimpleStruct::Type t;
        uint8_t buf[4]  = { 0, 1, 2, 3 };
        char strbuf[10] = "chip";

        t.a = 20;
        t.b = true;
        t.c = Clusters::UnitTesting::SimpleEnum::kValueA;
        t.d = buf;

        t.e = Span<char>{ strbuf, strlen(strbuf) };

        t.f.Set(Clusters::UnitTesting::SimpleBitmap::kValueC);
        t.g = 1.5f;
        t.h = 2.5;

        EXPECT_EQ(DataModel::Encode(mWriter, TLV::AnonymousTag(), t), CHIP_NO_ERROR);

        EXPECT_EQ(mWriter.Finalize(), CHIP_NO_ERROR);

        DumpBuf();
    }

    //
    // Decode
    //
    {
        Clusters::UnitTesting::Structs::SimpleStruct::Type t;

        SetupReader();

        EXPECT_EQ(DataModel::Decode(mReader, t), CHIP_NO_ERROR);

        EXPECT_EQ(t.a, 20);
        EXPECT_TRUE(t.b);
        EXPECT_EQ(t.c, Clusters::UnitTesting::SimpleEnum::kValueA);

        EXPECT_EQ(t.d.size(), 4u);

        for (uint32_t i = 0; i < t.d.size(); i++)
        {
            EXPECT_EQ(t.d.data()[i], i);
        }

        EXPECT_TRUE(StringMatches(t.e, "chip"));
        EXPECT_TRUE(t.f.HasOnly(Clusters::UnitTesting::SimpleBitmap::kValueC));
        EXPECT_EQ(t.g, 1.5f);
        EXPECT_EQ(t.h, 2.5);
        EXPECT_FALSE(t.i.HasValue());
    }
}

TEST_F(TestDataModelSerialization, CommandWithoutFieldsRoundTrips)
{
    SetupBuf();
    UnitTesting::Commands::Test::Type encoded;
    ASSERT_SUCCESS(DataModel::Encode(mWriter, TLV::AnonymousTag(), encoded));
    ASSERT_SUCCESS(mWriter.Finalize());
    SetupReader();
    ASSERT_FALSE(HasFailure());

    UnitTesting::Commands::Test::DecodableType decoded;
    EXPECT_SUCCESS(DataModel::Decode(mReader, decoded));
}

TEST_F(TestDataModelSerialization, OptionalCommandArgumentPreservesPresence)
{
    const std::optional<bool> arguments[] = { std::nullopt, false, true };
    for (const auto argument : arguments)
    {
        SetupBuf();
        UnitTesting::Commands::TestSimpleOptionalArgumentRequest::Type encoded;
        if (argument.has_value())
        {
            encoded.arg1.SetValue(*argument);
        }
        ASSERT_SUCCESS(DataModel::Encode(mWriter, TLV::AnonymousTag(), encoded));
        ASSERT_SUCCESS(mWriter.Finalize());
        SetupReader();
        ASSERT_FALSE(HasFailure());

        UnitTesting::Commands::TestSimpleOptionalArgumentRequest::DecodableType decoded;
        ASSERT_SUCCESS(DataModel::Decode(mReader, decoded));
        ASSERT_EQ(decoded.arg1.HasValue(), argument.has_value());
        if (argument.has_value())
        {
            EXPECT_EQ(decoded.arg1.Value(), *argument);
        }
    }
}

TEST_F(TestDataModelSerialization, FabricScopedStructPreservesEncodingModes)
{
    constexpr FabricIndex kOwnerFabric                  = 1;
    constexpr FabricIndex kOtherFabric                  = 2;
    const std::optional<FabricIndex> accessingFabrics[] = { std::nullopt, kOwnerFabric, kOtherFabric };
    const uint8_t values[]                              = { 7, 8 };

    for (const auto accessingFabric : accessingFabrics)
    {
        SetupBuf();
        UnitTesting::Structs::TestFabricScoped::Type encoded;
        encoded.SetFabricIndex(kOwnerFabric);
        encoded.fabricSensitiveInt8u      = 29;
        encoded.fabricSensitiveCharString = "fabric"_span;
        encoded.fabricSensitiveStruct.a   = 21;
        encoded.fabricSensitiveStruct.b   = true;
        encoded.fabricSensitiveStruct.c   = UnitTesting::SimpleEnum::kValueB;
        encoded.fabricSensitiveStruct.d   = ByteSpan(values);
        encoded.fabricSensitiveStruct.e   = "nested"_span;
        encoded.fabricSensitiveInt8uList  = DataModel::List<const uint8_t>(values);
        encoded.nullableFabricSensitiveInt8u.SetNull();

        if (accessingFabric.has_value())
        {
            ASSERT_SUCCESS(DataModel::EncodeForRead(mWriter, TLV::AnonymousTag(), *accessingFabric, encoded));
        }
        else
        {
            // Write encoding omits FabricIndex; the accessing fabric is supplied by the caller.
            ASSERT_SUCCESS(DataModel::EncodeForWrite(mWriter, TLV::AnonymousTag(), encoded));
        }
        ASSERT_SUCCESS(mWriter.Finalize());
        SetupReader();
        ASSERT_FALSE(HasFailure());

        const bool includeSensitive = !accessingFabric.has_value() || *accessingFabric == kOwnerFabric;
        // Check the encoder's actual field presence as well as the decoded values.
        TLV::TLVReader fields = mReader;
        TLV::TLVType outer;
        ASSERT_SUCCESS(fields.EnterContainer(outer));
        bool hasFabricIndex  = false;
        bool hasSensitiveInt = false;
        bool hasNullableInt  = false;
        CHIP_ERROR err;
        while ((err = fields.Next()) == CHIP_NO_ERROR)
        {
            hasFabricIndex |= fields.GetTag() == TLV::ContextTag(UnitTesting::Structs::TestFabricScoped::Fields::kFabricIndex);
            hasSensitiveInt |=
                fields.GetTag() == TLV::ContextTag(UnitTesting::Structs::TestFabricScoped::Fields::kFabricSensitiveInt8u);
            if (fields.GetTag() == TLV::ContextTag(UnitTesting::Structs::TestFabricScoped::Fields::kNullableFabricSensitiveInt8u))
            {
                hasNullableInt = true;
                EXPECT_EQ(fields.GetType(), TLV::kTLVType_Null);
            }
        }
        ASSERT_EQ(err, CHIP_ERROR_END_OF_TLV);
        ASSERT_SUCCESS(fields.ExitContainer(outer));
        EXPECT_EQ(hasFabricIndex, accessingFabric.has_value());
        EXPECT_EQ(hasSensitiveInt, includeSensitive);
        EXPECT_EQ(hasNullableInt, includeSensitive);

        UnitTesting::Structs::TestFabricScoped::DecodableType decoded;
        const auto context = accessingFabric.has_value() ? DataModel::DecodeContext::kRead : DataModel::DecodeContext::kWrite;
        ASSERT_SUCCESS(DataModel::Decode(mReader, decoded, context));
        EXPECT_EQ(decoded.GetFabricIndex(), accessingFabric.has_value() ? kOwnerFabric : kUndefinedFabricIndex);
        EXPECT_FALSE(decoded.optionalFabricSensitiveInt8u.HasValue());
        EXPECT_FALSE(decoded.nullableOptionalFabricSensitiveInt8u.HasValue());
        EXPECT_TRUE(decoded.nullableFabricSensitiveInt8u.IsNull());

        auto iterator = decoded.fabricSensitiveInt8uList.begin();
        if (includeSensitive)
        {
            EXPECT_EQ(decoded.fabricSensitiveInt8u, encoded.fabricSensitiveInt8u);
            EXPECT_TRUE(decoded.fabricSensitiveCharString.data_equal(encoded.fabricSensitiveCharString));
            EXPECT_EQ(decoded.fabricSensitiveStruct.a, encoded.fabricSensitiveStruct.a);
            EXPECT_TRUE(decoded.fabricSensitiveStruct.d.data_equal(encoded.fabricSensitiveStruct.d));
            for (const auto value : values)
            {
                ASSERT_TRUE(iterator.Next());
                EXPECT_EQ(iterator.GetValue(), value);
            }
        }
        else
        {
            // Reads from another fabric omit the sensitive fields while retaining FabricIndex.
            EXPECT_EQ(decoded.fabricSensitiveInt8u, 0);
            EXPECT_TRUE(decoded.fabricSensitiveCharString.empty());
            EXPECT_EQ(decoded.fabricSensitiveStruct.a, 0);
        }
        EXPECT_FALSE(iterator.Next());
        EXPECT_SUCCESS(iterator.GetStatus());
    }
}

TEST_F(TestDataModelSerialization, DecodeContextReachesOptionalNullableValue)
{
    for (const auto context : { DataModel::DecodeContext::kRead, DataModel::DecodeContext::kWrite })
    {
        SetupBuf();
        ASSERT_SUCCESS(DataModel::Encode(mWriter, TLV::AnonymousTag(), uint8_t{ 7 }));
        ASSERT_SUCCESS(mWriter.Finalize());
        SetupReader();
        ASSERT_FALSE(HasFailure());

        Optional<DataModel::Nullable<ContextRecordingValue>> decoded;
        ASSERT_SUCCESS(DataModel::Decode(mReader, decoded, context));
        ASSERT_TRUE(decoded.HasValue());
        ASSERT_FALSE(decoded.Value().IsNull());
        EXPECT_EQ(decoded.Value().Value().value, 7);
        EXPECT_EQ(decoded.Value().Value().context, context);
    }
}

TEST_F(TestDataModelSerialization, DecodeContextReachesLazyListElements)
{
    const uint8_t values[] = { 7, 8 };
    DataModel::DecodableList<ContextRecordingValue> decoded;
    for (const auto context : { DataModel::DecodeContext::kRead, DataModel::DecodeContext::kWrite })
    {
        SetupBuf();
        ASSERT_SUCCESS(DataModel::Encode(mWriter, TLV::AnonymousTag(), DataModel::List<const uint8_t>(values)));
        ASSERT_SUCCESS(mWriter.Finalize());
        SetupReader();
        ASSERT_FALSE(HasFailure());

        ASSERT_SUCCESS(DataModel::Decode(mReader, decoded, context));
        auto iterator = decoded.begin();
        for (const auto value : values)
        {
            ASSERT_TRUE(iterator.Next());
            EXPECT_EQ(iterator.GetValue().value, value);
            EXPECT_EQ(iterator.GetValue().context, context);
        }
        EXPECT_FALSE(iterator.Next());
        EXPECT_SUCCESS(iterator.GetStatus());
    }
}

TEST_F(TestDataModelSerialization, EncAndDecSimpleStructNegativeEnum)

{

    SetupBuf();

    //
    // Encode
    //
    {
        Clusters::UnitTesting::Structs::SimpleStruct::Type t;
        uint8_t buf[4]  = { 0, 1, 2, 3 };
        char strbuf[10] = "chip";

        t.a = 20;
        t.b = true;
        t.c = static_cast<Clusters::UnitTesting::SimpleEnum>(10);
        t.d = buf;

        t.e = Span<char>{ strbuf, strlen(strbuf) };

        t.f.Set(Clusters::UnitTesting::SimpleBitmap::kValueC);

        EXPECT_EQ(DataModel::Encode(mWriter, TLV::AnonymousTag(), t), CHIP_NO_ERROR);

        EXPECT_EQ(mWriter.Finalize(), CHIP_NO_ERROR);

        DumpBuf();
    }

    //
    // Decode
    //
    {
        Clusters::UnitTesting::Structs::SimpleStruct::Type t;

        SetupReader();

        EXPECT_EQ(DataModel::Decode(mReader, t), CHIP_NO_ERROR);
        EXPECT_EQ(to_underlying(t.c), 4);
    }
}

TEST_F(TestDataModelSerialization, EncAndDecNestedStruct)
{

    SetupBuf();

    //
    // Encode
    //
    {
        Clusters::UnitTesting::Structs::NestedStruct::Type t;
        uint8_t buf[4]  = { 0, 1, 2, 3 };
        char strbuf[10] = "chip";

        t.a   = 20;
        t.b   = true;
        t.c.a = 11;
        t.c.b = true;
        t.c.c = Clusters::UnitTesting::SimpleEnum::kValueB;
        t.c.d = buf;

        t.c.e = Span<char>{ strbuf, strlen(strbuf) };

        EXPECT_EQ(DataModel::Encode(mWriter, TLV::AnonymousTag(), t), CHIP_NO_ERROR);
        EXPECT_EQ(mWriter.Finalize(), CHIP_NO_ERROR);

        DumpBuf();
    }

    //
    // Decode
    //
    {
        Clusters::UnitTesting::Structs::NestedStruct::DecodableType t;

        SetupReader();

        EXPECT_EQ(DataModel::Decode(mReader, t), CHIP_NO_ERROR);

        EXPECT_EQ(t.a, 20);
        EXPECT_TRUE(t.b);
        EXPECT_EQ(t.c.a, 11);
        EXPECT_TRUE(t.c.b);
        EXPECT_EQ(t.c.c, Clusters::UnitTesting::SimpleEnum::kValueB);

        EXPECT_EQ(t.c.d.size(), 4u);

        for (uint32_t i = 0; i < t.c.d.size(); i++)
        {
            EXPECT_EQ(t.c.d.data()[i], i);
        }

        EXPECT_TRUE(StringMatches(t.c.e, "chip"));
    }
}
TEST_F(TestDataModelSerialization, EncAndDecDecodableNestedStructList)
{

    SetupBuf();

    //
    // Encode
    //
    {
        Clusters::UnitTesting::Structs::NestedStructList::Type t;
        uint8_t buf[4]     = { 0, 1, 2, 3 };
        uint32_t intBuf[4] = { 10000, 10001, 10002, 10003 };
        char strbuf[10]    = "chip";
        Clusters::UnitTesting::Structs::SimpleStruct::Type structList[4];
        uint8_t i = 0;
        ByteSpan spanList[4];

        t.a   = 20;
        t.b   = true;
        t.c.a = 11;
        t.c.b = true;
        t.c.c = Clusters::UnitTesting::SimpleEnum::kValueB;
        t.c.d = buf;
        t.e   = intBuf;

        for (auto & item : structList)
        {
            item.a = i;
            item.b = true;
            i++;
        }

        t.f = spanList;

        spanList[0] = buf;
        spanList[1] = buf;
        spanList[2] = buf;
        spanList[3] = buf;

        t.g = buf;

        t.c.e = Span<char>{ strbuf, strlen(strbuf) };
        t.d   = structList;

        EXPECT_EQ(DataModel::Encode(mWriter, TLV::AnonymousTag(), t), CHIP_NO_ERROR);
        EXPECT_EQ(mWriter.Finalize(), CHIP_NO_ERROR);

        DumpBuf();
    }

    //
    // Decode
    //
    {
        Clusters::UnitTesting::Structs::NestedStructList::DecodableType t;
        int i;

        SetupReader();

        EXPECT_EQ(DataModel::Decode(mReader, t), CHIP_NO_ERROR);

        EXPECT_EQ(t.a, 20);
        EXPECT_TRUE(t.b);
        EXPECT_EQ(t.c.a, 11);
        EXPECT_TRUE(t.c.b);
        EXPECT_EQ(t.c.c, Clusters::UnitTesting::SimpleEnum::kValueB);

        EXPECT_TRUE(StringMatches(t.c.e, "chip"));

        {
            i         = 0;
            auto iter = t.d.begin();
            while (iter.Next())
            {
                auto & item = iter.GetValue();
                EXPECT_EQ(item.a, static_cast<uint8_t>(i));
                EXPECT_TRUE(item.b);
                i++;
            }

            EXPECT_EQ(iter.GetStatus(), CHIP_NO_ERROR);
            EXPECT_EQ(i, 4);
        }

        {
            i         = 0;
            auto iter = t.e.begin();
            while (iter.Next())
            {
                auto & item = iter.GetValue();
                EXPECT_EQ(item, static_cast<uint32_t>(i + 10000));
                i++;
            }

            EXPECT_EQ(iter.GetStatus(), CHIP_NO_ERROR);
            EXPECT_EQ(i, 4);
        }

        {
            i         = 0;
            auto iter = t.f.begin();

            while (iter.Next())
            {
                auto & item = iter.GetValue();

                unsigned int j = 0;
                for (; j < item.size(); j++)
                {
                    EXPECT_EQ(item.data()[j], j);
                }

                EXPECT_EQ(j, 4u);
                i++;
            }

            EXPECT_EQ(iter.GetStatus(), CHIP_NO_ERROR);
            EXPECT_EQ(i, 4);
        }

        {
            i         = 0;
            auto iter = t.g.begin();

            while (iter.Next())
            {
                auto & item = iter.GetValue();
                EXPECT_EQ(item, i);
                i++;
            }

            EXPECT_EQ(iter.GetStatus(), CHIP_NO_ERROR);
            EXPECT_EQ(i, 4);
        }
    }
}
TEST_F(TestDataModelSerialization, EncAndDecDecodableDoubleNestedStructList)
{

    SetupBuf();

    //
    // Encode
    //
    {
        Clusters::UnitTesting::Structs::DoubleNestedStructList::Type t;
        Clusters::UnitTesting::Structs::NestedStructList::Type n[4];
        Clusters::UnitTesting::Structs::SimpleStruct::Type structList[4];
        uint8_t i;

        t.a = n;

        i = 0;
        for (auto & item : structList)
        {
            item.a = static_cast<uint8_t>(35 + i);
            i++;
        }

        for (auto & item : n)
        {
            item.d = structList;
        }

        EXPECT_EQ(DataModel::Encode(mWriter, TLV::AnonymousTag(), t), CHIP_NO_ERROR);
        EXPECT_EQ(mWriter.Finalize(), CHIP_NO_ERROR);

        DumpBuf();
    }

    //
    // Decode
    //
    {
        Clusters::UnitTesting::Structs::DoubleNestedStructList::DecodableType t;

        SetupReader();

        EXPECT_EQ(DataModel::Decode(mReader, t), CHIP_NO_ERROR);

        uint8_t i = 0;

        auto iter = t.a.begin();
        while (iter.Next())
        {
            auto & item     = iter.GetValue();
            auto nestedIter = item.d.begin();

            unsigned int j = 0;
            while (nestedIter.Next())
            {
                auto & nestedItem = nestedIter.GetValue();

                EXPECT_EQ(nestedItem.a, (static_cast<uint8_t>(35) + j));
                j++;
            }

            EXPECT_EQ(j, 4u);
            i++;
        }

        EXPECT_EQ(iter.GetStatus(), CHIP_NO_ERROR);
        EXPECT_EQ(i, 4u);
    }
}

TEST_F(TestDataModelSerialization, OptionalStructFieldMayBeOmitted)
{

    SetupBuf();

    //
    // Encode
    //
    {
        Clusters::UnitTesting::Structs::SimpleStruct::Type t;
        uint8_t buf[4]  = { 0, 1, 2, 3 };
        char strbuf[10] = "chip";

        t.a = 20;
        t.b = true;
        t.c = Clusters::UnitTesting::SimpleEnum::kValueA;
        t.d = buf;

        t.e = Span<char>{ strbuf, strlen(strbuf) };

        // Encode all required fields; the optional field i is absent.
        {
            EXPECT_EQ(
                EncodeStruct(mWriter, TLV::AnonymousTag(),
                             MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kA), t.a),
                             MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kB), t.b),
                             MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kC), t.c),
                             MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kD), t.d),
                             MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kE), t.e),
                             MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kF), t.f),
                             MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kG), t.g),
                             MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kH), t.h)),
                CHIP_NO_ERROR);
        }

        EXPECT_EQ(mWriter.Finalize(), CHIP_NO_ERROR);

        DumpBuf();
    }

    //
    // Decode
    //
    {
        Clusters::UnitTesting::Structs::SimpleStruct::DecodableType t;

        SetupReader();

        // An absent optional field retains its existing value.
        t.i.SetValue(Globals::TestGlobalEnum::kSomeOtherValue);

        EXPECT_EQ(DataModel::Decode(mReader, t), CHIP_NO_ERROR);

        EXPECT_EQ(t.a, 20);
        ASSERT_TRUE(t.i.HasValue());
        EXPECT_EQ(t.i.Value(), Globals::TestGlobalEnum::kSomeOtherValue);

        EXPECT_TRUE(t.b);
        EXPECT_EQ(t.c, Clusters::UnitTesting::SimpleEnum::kValueA);

        EXPECT_EQ(t.d.size(), 4u);

        for (uint32_t i = 0; i < t.d.size(); i++)
        {
            EXPECT_EQ(t.d.data()[i], i);
        }

        EXPECT_TRUE(StringMatches(t.e, "chip"));
    }
}

TEST_F(TestDataModelSerialization, ExtraField)
{
    SetupBuf();

    //
    // Encode
    //
    {
        Clusters::UnitTesting::Structs::SimpleStruct::Type t;
        uint8_t buf[4]  = { 0, 1, 2, 3 };
        char strbuf[10] = "chip";

        t.a = 20;
        t.b = true;
        t.c = Clusters::UnitTesting::SimpleEnum::kValueA;
        t.d = buf;

        t.e = Span<char>{ strbuf, strlen(strbuf) };

        // Encode every field + an extra field.
        {
            EXPECT_EQ(EncodeStruct(mWriter, TLV::AnonymousTag(),
                                   MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kA), t.a),
                                   MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kB), t.b),
                                   MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kC), t.c),
                                   MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kD), t.d),
                                   MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kE), t.e),
                                   MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kF), t.f),
                                   MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kG), t.g),
                                   MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kH), t.h),
                                   MakeTagValuePair(TLV::ContextTag(255), t.a)),
                      CHIP_NO_ERROR);
        }

        EXPECT_EQ(mWriter.Finalize(), CHIP_NO_ERROR);

        DumpBuf();
    }

    //
    // Decode
    //
    {
        Clusters::UnitTesting::Structs::SimpleStruct::DecodableType t;

        SetupReader();

        // Ensure successful decode despite the extra field.
        EXPECT_EQ(DataModel::Decode(mReader, t), CHIP_NO_ERROR);

        EXPECT_EQ(t.a, 20);
        EXPECT_TRUE(t.b);
        EXPECT_EQ(t.c, Clusters::UnitTesting::SimpleEnum::kValueA);

        EXPECT_EQ(t.d.size(), 4u);

        for (uint32_t i = 0; i < t.d.size(); i++)
        {
            EXPECT_EQ(t.d.data()[i], i);
        }

        EXPECT_TRUE(StringMatches(t.e, "chip"));
    }
}

TEST_F(TestDataModelSerialization, InvalidSimpleFieldTypes)
{
    SetupBuf();
    //
    // Case #1: Swap out field a (an integer) with a boolean.
    //
    {
        //
        // Encode
        //
        {
            Clusters::UnitTesting::Structs::SimpleStruct::Type t;
            uint8_t buf[4]  = { 0, 1, 2, 3 };
            char strbuf[10] = "chip";

            t.a = 20;
            t.b = true;
            t.c = Clusters::UnitTesting::SimpleEnum::kValueA;
            t.d = buf;

            t.e = Span<char>{ strbuf, strlen(strbuf) };

            // Encode every field manually except a.
            {
                EXPECT_EQ(
                    EncodeStruct(mWriter, TLV::AnonymousTag(),
                                 MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kA), t.b),
                                 MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kB), t.b),
                                 MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kC), t.c),
                                 MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kD), t.d),
                                 MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kE), t.e)),
                    CHIP_NO_ERROR);
            }

            EXPECT_EQ(mWriter.Finalize(), CHIP_NO_ERROR);
            DumpBuf();
        }
        //
        // Decode
        //
        {
            Clusters::UnitTesting::Structs::SimpleStruct::DecodableType t;

            SetupReader();

            EXPECT_NE(DataModel::Decode(mReader, t), CHIP_NO_ERROR);
        }
    }

    SetupBuf();
    //
    // Case #2: Swap out an octet string with a UTF-8 string.
    //
    {
        //
        // Encode
        //
        {
            Clusters::UnitTesting::Structs::SimpleStruct::Type t;
            uint8_t buf[4]  = { 0, 1, 2, 3 };
            char strbuf[10] = "chip";

            t.a = 20;
            t.b = true;
            t.c = Clusters::UnitTesting::SimpleEnum::kValueA;
            t.d = buf;

            t.e = Span<char>{ strbuf, strlen(strbuf) };

            // Encode every field manually except a.
            {
                EXPECT_EQ(
                    EncodeStruct(mWriter, TLV::AnonymousTag(),
                                 MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kA), t.a),
                                 MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kB), t.b),
                                 MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kC), t.c),
                                 MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kD), t.e),
                                 MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kE), t.e)),
                    CHIP_NO_ERROR);
            }

            EXPECT_EQ(mWriter.Finalize(), CHIP_NO_ERROR);

            DumpBuf();
        }

        //
        // Decode
        //
        {
            Clusters::UnitTesting::Structs::SimpleStruct::DecodableType t;

            SetupReader();

            EXPECT_NE(DataModel::Decode(mReader, t), CHIP_NO_ERROR);
        }
    }
}

TEST_F(TestDataModelSerialization, InvalidListType)
{
    SetupBuf();

    //
    // Encode
    //
    {
        Clusters::UnitTesting::Structs::NestedStructList::Type t;
        uint32_t intBuf[4] = { 10000, 10001, 10002, 10003 };

        t.e = intBuf;

        // Encode a list of integers for field d instead of a list of structs.
        {
            EXPECT_EQ(
                EncodeStruct(mWriter, TLV::AnonymousTag(),
                             MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::NestedStructList::Fields::kA), t.a),
                             MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::NestedStructList::Fields::kB), t.b),
                             MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::NestedStructList::Fields::kC), t.c),
                             MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::NestedStructList::Fields::kD), t.e),
                             MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::NestedStructList::Fields::kE), t.e),
                             MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::NestedStructList::Fields::kF), t.f),
                             MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::NestedStructList::Fields::kG), t.g)),
                CHIP_NO_ERROR);
        }

        EXPECT_EQ(mWriter.Finalize(), CHIP_NO_ERROR);
        DumpBuf();
    }

    //
    // Decode
    //
    {
        Clusters::UnitTesting::Structs::NestedStructList::DecodableType t;

        SetupReader();

        EXPECT_EQ(DataModel::Decode(mReader, t), CHIP_NO_ERROR);

        auto iter     = t.d.begin();
        bool hadItems = false;

        while (iter.Next())
        {
            hadItems = true;
        }

        EXPECT_NE(iter.GetStatus(), CHIP_NO_ERROR);
        EXPECT_FALSE(hadItems);
    }
}

namespace {
bool SimpleStructsEqual(const Clusters::UnitTesting::Structs::SimpleStruct::Type & s1,
                        const Clusters::UnitTesting::Structs::SimpleStruct::Type & s2)
{
    return s1.a == s2.a && s1.b == s2.b && s1.c == s2.c && s1.d.data_equal(s2.d) && s1.e.data_equal(s2.e) && s1.f == s2.f;
}

template <typename T>
bool ListsEqual(const DataModel::DecodableList<T> & list1, const DataModel::List<T> & list2)
{
    auto iter1 = list1.begin();
    auto iter2 = list2.begin();
    auto end2  = list2.end();
    while (iter1.Next())
    {
        if (iter2 == end2)
        {
            // list2 too small
            return false;
        }

        if (iter1.GetValue() != *iter2)
        {
            return false;
        }
        ++iter2;
    }
    if (iter1.GetStatus() != CHIP_NO_ERROR)
    {
        // Failed to decode
        return false;
    }
    if (iter2 != end2)
    {
        // list1 too small
        return false;
    }
    return true;
}

} // anonymous namespace

template <typename Encodable, typename Decodable>
void TestDataModelSerialization::NullablesOptionalsEncodeDecodeCheck(bool encodeNulls, bool encodeValues)
{

    SetupBuf();

    static const char structStr[] = "something";
    const uint8_t structBytes[]   = { 1, 8, 17 };
    Clusters::UnitTesting::Structs::SimpleStruct::Type myStruct;
    myStruct.a = 17;
    myStruct.b = true;
    myStruct.c = Clusters::UnitTesting::SimpleEnum::kValueB;
    myStruct.d = ByteSpan(structBytes);
    myStruct.e = CharSpan::fromCharString(structStr);
    myStruct.f = Clusters::UnitTesting::SimpleBitmap(2);

    Clusters::UnitTesting::SimpleEnum enumListVals[] = { Clusters::UnitTesting::SimpleEnum::kValueA,
                                                         Clusters::UnitTesting::SimpleEnum::kValueC };
    DataModel::List<Clusters::UnitTesting::SimpleEnum> enumList(enumListVals);

    // Encode
    {
        // str needs to live until we call DataModel::Encode.
        static const char str[] = "abc";
        CharSpan strSpan        = CharSpan::fromCharString(str);
        Encodable encodable;
        if (encodeNulls)
        {
            encodable.nullableInt.SetNull();
            encodable.nullableOptionalInt.Emplace().SetNull();

            encodable.nullableString.SetNull();
            encodable.nullableOptionalString.Emplace().SetNull();

            encodable.nullableStruct.SetNull();
            encodable.nullableOptionalStruct.Emplace().SetNull();

            encodable.nullableList.SetNull();
            encodable.nullableOptionalList.Emplace().SetNull();
        }
        else if (encodeValues)
        {
            encodable.nullableInt.SetNonNull(static_cast<uint16_t>(5u));
            encodable.optionalInt.Emplace(static_cast<uint16_t>(6u));
            encodable.nullableOptionalInt.Emplace().SetNonNull() = 7;

            encodable.nullableString.SetNonNull(strSpan);
            encodable.optionalString.Emplace() = strSpan;
            encodable.nullableOptionalString.Emplace().SetNonNull(strSpan);

            encodable.nullableStruct.SetNonNull(myStruct);
            encodable.optionalStruct.Emplace(myStruct);
            encodable.nullableOptionalStruct.Emplace().SetNonNull(myStruct);

            encodable.nullableList.SetNonNull() = enumList;
            encodable.optionalList.Emplace(enumList);
            encodable.nullableOptionalList.Emplace().SetNonNull(enumList);
        }
        else
        {
            // Just encode the non-optionals, as null.
            encodable.nullableInt.SetNull();
            encodable.nullableString.SetNull();
            encodable.nullableStruct.SetNull();
            encodable.nullableList.SetNull();
        }

        EXPECT_SUCCESS(DataModel::Encode(mWriter, TLV::AnonymousTag(), encodable));
        EXPECT_EQ(mWriter.Finalize(), CHIP_NO_ERROR);
    }

    // Decode
    {
        SetupReader();

        Decodable decodable;
        EXPECT_EQ(DataModel::Decode(mReader, decodable), CHIP_NO_ERROR);

        if (encodeNulls)
        {
            EXPECT_TRUE(decodable.nullableInt.IsNull());
            EXPECT_FALSE(decodable.optionalInt.HasValue());
            EXPECT_TRUE(decodable.nullableOptionalInt.HasValue());
            EXPECT_TRUE(decodable.nullableOptionalInt.Value().IsNull());

            EXPECT_TRUE(decodable.nullableString.IsNull());
            EXPECT_FALSE(decodable.optionalString.HasValue());
            EXPECT_TRUE(decodable.nullableOptionalString.HasValue());
            EXPECT_TRUE(decodable.nullableOptionalString.Value().IsNull());

            EXPECT_TRUE(decodable.nullableStruct.IsNull());
            EXPECT_FALSE(decodable.optionalStruct.HasValue());
            EXPECT_TRUE(decodable.nullableOptionalStruct.HasValue());
            EXPECT_TRUE(decodable.nullableOptionalStruct.Value().IsNull());

            EXPECT_TRUE(decodable.nullableList.IsNull());
            EXPECT_FALSE(decodable.optionalList.HasValue());
            EXPECT_TRUE(decodable.nullableOptionalList.HasValue());
            EXPECT_TRUE(decodable.nullableOptionalList.Value().IsNull());
        }
        else if (encodeValues)
        {
            static const char str[] = "abc";
            CharSpan strSpan        = CharSpan::fromCharString(str);

            EXPECT_FALSE(decodable.nullableInt.IsNull());
            EXPECT_EQ(decodable.nullableInt.Value(), 5);
            EXPECT_TRUE(decodable.optionalInt.HasValue());
            EXPECT_EQ(decodable.optionalInt.Value(), 6);
            EXPECT_TRUE(decodable.nullableOptionalInt.HasValue());
            EXPECT_FALSE(decodable.nullableOptionalInt.Value().IsNull());
            EXPECT_EQ(decodable.nullableOptionalInt.Value().Value(), 7);

            EXPECT_FALSE(decodable.nullableString.IsNull());
            EXPECT_TRUE(decodable.nullableString.Value().data_equal(strSpan));
            EXPECT_TRUE(decodable.optionalString.HasValue());
            EXPECT_TRUE(decodable.optionalString.Value().data_equal(strSpan));
            EXPECT_TRUE(decodable.nullableOptionalString.HasValue());
            EXPECT_FALSE(decodable.nullableOptionalString.Value().IsNull());
            EXPECT_TRUE(decodable.nullableOptionalString.Value().Value().data_equal(strSpan));

            EXPECT_FALSE(decodable.nullableStruct.IsNull());
            EXPECT_TRUE(SimpleStructsEqual(decodable.nullableStruct.Value(), myStruct));
            EXPECT_TRUE(decodable.optionalStruct.HasValue());
            EXPECT_TRUE(SimpleStructsEqual(decodable.optionalStruct.Value(), myStruct));
            EXPECT_TRUE(decodable.nullableOptionalStruct.HasValue());
            EXPECT_FALSE(decodable.nullableOptionalStruct.Value().IsNull());
            EXPECT_TRUE(SimpleStructsEqual(decodable.nullableOptionalStruct.Value().Value(), myStruct));

            EXPECT_FALSE(decodable.nullableList.IsNull());
            EXPECT_TRUE(ListsEqual(decodable.nullableList.Value(), enumList));
            EXPECT_TRUE(decodable.optionalList.HasValue());
            EXPECT_TRUE(ListsEqual(decodable.optionalList.Value(), enumList));
            EXPECT_TRUE(decodable.nullableOptionalList.HasValue());
            EXPECT_FALSE(decodable.nullableOptionalList.Value().IsNull());
            EXPECT_TRUE(ListsEqual(decodable.nullableOptionalList.Value().Value(), enumList));
        }
        else
        {
            EXPECT_TRUE(decodable.nullableInt.IsNull());
            EXPECT_FALSE(decodable.optionalInt.HasValue());
            EXPECT_FALSE(decodable.nullableOptionalInt.HasValue());

            EXPECT_TRUE(decodable.nullableString.IsNull());
            EXPECT_FALSE(decodable.optionalString.HasValue());
            EXPECT_FALSE(decodable.nullableOptionalString.HasValue());

            EXPECT_TRUE(decodable.nullableStruct.IsNull());
            EXPECT_FALSE(decodable.optionalStruct.HasValue());
            EXPECT_FALSE(decodable.nullableOptionalStruct.HasValue());

            EXPECT_TRUE(decodable.nullableList.IsNull());
            EXPECT_FALSE(decodable.optionalList.HasValue());
            EXPECT_FALSE(decodable.nullableOptionalList.HasValue());
        }
    }
}

template <typename Encodable, typename Decodable>
void TestDataModelSerialization::NullablesOptionalsEncodeDecodeCheck()
{
    NullablesOptionalsEncodeDecodeCheck<Encodable, Decodable>(false, false);
    NullablesOptionalsEncodeDecodeCheck<Encodable, Decodable>(true, false);
    NullablesOptionalsEncodeDecodeCheck<Encodable, Decodable>(false, true);
}

TEST_F(TestDataModelSerialization, NullablesOptionalsStruct)
{
    using EncType = Clusters::UnitTesting::Structs::NullablesAndOptionalsStruct::Type;
    using DecType = Clusters::UnitTesting::Structs::NullablesAndOptionalsStruct::DecodableType;
    NullablesOptionalsEncodeDecodeCheck<EncType, DecType>();
}

TEST_F(TestDataModelSerialization, NullablesOptionalsCommand)
{
    using EncType = Clusters::UnitTesting::Commands::TestComplexNullableOptionalRequest::Type;
    using DecType = Clusters::UnitTesting::Commands::TestComplexNullableOptionalRequest::DecodableType;
    NullablesOptionalsEncodeDecodeCheck<EncType, DecType>();
}

} // namespace
