#include "cluster-objects.h"

#pragma GCC diagnostic ignored "-Wstack-usage="

namespace chip {
namespace app {
namespace clusters {
namespace TestCluster {
namespace TestListStructOctet {
CHIP_ERROR Type::Encode(TLV::TLVWriter & writer, uint64_t tag)
{
    TLV::TLVType outer;
    ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));
    ReturnErrorOnFailure(ClusterObjectUtils::Encode(writer, TLV::ContextTag(kFabricIndexFieldId), fabricIndex));
    ReturnErrorOnFailure(ClusterObjectUtils::Encode(writer, TLV::ContextTag(kOperationalCertFieldId), operationalCert));
    ReturnErrorOnFailure(writer.EndContainer(outer));
    return CHIP_NO_ERROR;
}

CHIP_ERROR Type::Decode(TLV::TLVReader & reader)
{
    CHIP_ERROR err;
    TLV::TLVType outer;

    err = reader.EnterContainer(outer);
    ReturnErrorOnFailure(err);

    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        uint64_t tag = reader.GetTag();

        if (tag == TLV::ContextTag(kFabricIndexFieldId))
        {
            ReturnErrorOnFailure(ClusterObjectUtils::Decode(reader, fabricIndex));
        }
        else if (tag == TLV::ContextTag(kOperationalCertFieldId))
        {
            ReturnErrorOnFailure(ClusterObjectUtils::Decode(reader, operationalCert));
        }
    }

    ReturnErrorOnFailure(reader.ExitContainer(outer));
    return CHIP_NO_ERROR;
}
} // namespace TestListStructOctet

namespace SimpleStruct {
CHIP_ERROR Type::Encode(TLV::TLVWriter & writer, uint64_t tag)
{
    TLV::TLVType outer;

    ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));
    ReturnErrorOnFailure(ClusterObjectUtils::Encode(writer, TLV::ContextTag(kAFieldId), a));
    ReturnErrorOnFailure(ClusterObjectUtils::Encode(writer, TLV::ContextTag(kBFieldId), b));
    ReturnErrorOnFailure(ClusterObjectUtils::Encode(writer, TLV::ContextTag(kCFieldId), c));
    ReturnErrorOnFailure(ClusterObjectUtils::Encode(writer, TLV::ContextTag(kDFieldId), d));
    ReturnErrorOnFailure(ClusterObjectUtils::Encode(writer, TLV::ContextTag(kEFieldId), e));
    ReturnErrorOnFailure(writer.EndContainer(outer));

    return CHIP_NO_ERROR;
}

CHIP_ERROR Type::Decode(TLV::TLVReader & reader)
{
    CHIP_ERROR err;
    TLV::TLVType outer;

    err = reader.EnterContainer(outer);
    ReturnErrorOnFailure(err);

    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        uint64_t tag = reader.GetTag();

        if (tag == TLV::ContextTag(kAFieldId))
        {
            ReturnErrorOnFailure(ClusterObjectUtils::Decode(reader, a));
        }
        else if (tag == TLV::ContextTag(kBFieldId))
        {
            ReturnErrorOnFailure(ClusterObjectUtils::Decode(reader, b));
        }
        else if (tag == TLV::ContextTag(kCFieldId))
        {
            uint8_t v;
            ReturnErrorOnFailure(ClusterObjectUtils::Decode(reader, v));
            c = (SimpleEnum) v;
        }
        else if (tag == TLV::ContextTag(kDFieldId))
        {
            ReturnErrorOnFailure(ClusterObjectUtils::Decode(reader, d));
        }
        else if (tag == TLV::ContextTag(kEFieldId))
        {
            ReturnErrorOnFailure(ClusterObjectUtils::Decode(reader, e));
        }
    }

    ReturnErrorOnFailure(reader.ExitContainer(outer));
    return CHIP_NO_ERROR;
}
} // namespace SimpleStruct

namespace NestedStruct {
CHIP_ERROR Type::Encode(TLV::TLVWriter & writer, uint64_t tag)
{
    TLV::TLVType outer;

    ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));
    ReturnErrorOnFailure(ClusterObjectUtils::Encode(writer, TLV::ContextTag(kAFieldId), a));
    ReturnErrorOnFailure(ClusterObjectUtils::Encode(writer, TLV::ContextTag(kBFieldId), b));
    ReturnErrorOnFailure(ClusterObjectUtils::Encode(writer, TLV::ContextTag(kCFieldId), c));
    ReturnErrorOnFailure(writer.EndContainer(outer));

    return CHIP_NO_ERROR;
}

CHIP_ERROR Type::Decode(TLV::TLVReader & reader)
{
    CHIP_ERROR err;
    TLV::TLVType outer;

    err = reader.EnterContainer(outer);
    ReturnErrorOnFailure(err);

    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        uint64_t tag = reader.GetTag();

        if (tag == TLV::ContextTag(kAFieldId))
        {
            ReturnErrorOnFailure(ClusterObjectUtils::Decode(reader, a));
        }
        else if (tag == TLV::ContextTag(kBFieldId))
        {
            ReturnErrorOnFailure(ClusterObjectUtils::Decode(reader, b));
        }
        else if (tag == TLV::ContextTag(kCFieldId))
        {
            ReturnErrorOnFailure(ClusterObjectUtils::Decode(reader, c));
        }
    }

    ReturnErrorOnFailure(reader.ExitContainer(outer));
    return CHIP_NO_ERROR;
}
} // namespace NestedStruct

namespace NestedStructList {
CHIP_ERROR Type::Encode(TLV::TLVWriter & writer, uint64_t tag)
{
    TLV::TLVType outer;

    ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));

    {
        TLV::TLVType outer1;

        ReturnErrorOnFailure(ClusterObjectUtils::Encode(writer, TLV::ContextTag(kAFieldId), a));
        ReturnErrorOnFailure(ClusterObjectUtils::Encode(writer, TLV::ContextTag(kBFieldId), b));
        ReturnErrorOnFailure(ClusterObjectUtils::Encode(writer, TLV::ContextTag(kCFieldId), c));

        ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(kDFieldId), TLV::kTLVType_Array, outer1));
        for (auto & item : d)
        {
            ReturnErrorOnFailure(ClusterObjectUtils::Encode(writer, TLV::AnonymousTag, item));
        }
        ReturnErrorOnFailure(writer.EndContainer(outer1));

        ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(kEFieldId), TLV::kTLVType_Array, outer1));
        for (auto & item : e)
        {
            ReturnErrorOnFailure(ClusterObjectUtils::Encode(writer, TLV::AnonymousTag, item));
        }
        ReturnErrorOnFailure(writer.EndContainer(outer1));

        ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(kFFieldId), TLV::kTLVType_Array, outer1));
        for (auto & item : f)
        {
            ReturnErrorOnFailure(ClusterObjectUtils::Encode(writer, TLV::AnonymousTag, item));
        }
        ReturnErrorOnFailure(writer.EndContainer(outer1));
    }

    ReturnErrorOnFailure(writer.EndContainer(outer));

    return CHIP_NO_ERROR;
}

CHIP_ERROR Type::Decode(TLV::TLVReader & reader)
{
    CHIP_ERROR err;
    TLV::TLVType outer;

    err = reader.EnterContainer(outer);
    ReturnErrorOnFailure(err);

    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        uint64_t tag = reader.GetTag();

        if (tag == TLV::ContextTag(kAFieldId))
        {
            ReturnErrorOnFailure(ClusterObjectUtils::Decode(reader, a));
        }
        else if (tag == TLV::ContextTag(kBFieldId))
        {
            ReturnErrorOnFailure(ClusterObjectUtils::Decode(reader, b));
        }
        else if (tag == TLV::ContextTag(kCFieldId))
        {
            ReturnErrorOnFailure(ClusterObjectUtils::Decode(reader, c));
        }
        else if (tag == TLV::ContextTag(kDFieldId))
        {
            TLV::TLVType outer1;
            size_t destBufSize = d.size();
            uint32_t i         = 0;

            ReturnErrorOnFailure(reader.EnterContainer(outer1));

            while ((err = reader.Next()) == CHIP_NO_ERROR)
            {
                VerifyOrReturnError(destBufSize, CHIP_ERROR_BUFFER_TOO_SMALL);
                ReturnErrorOnFailure(ClusterObjectUtils::Decode(reader, d.data()[i]));
                destBufSize--;
            }

            ReturnErrorOnFailure(reader.ExitContainer(outer1));
        }
        else if (tag == TLV::ContextTag(kEFieldId))
        {
            TLV::TLVType outer1;
            size_t destBufSize = e.size();
            uint32_t i         = 0;

            ReturnErrorOnFailure(reader.EnterContainer(outer1));

            while ((err = reader.Next()) == CHIP_NO_ERROR)
            {
                VerifyOrReturnError(destBufSize, CHIP_ERROR_BUFFER_TOO_SMALL);
                ReturnErrorOnFailure(ClusterObjectUtils::Decode(reader, e.data()[i]));
                destBufSize--;
            }

            ReturnErrorOnFailure(reader.ExitContainer(outer1));
        }
        else if (tag == TLV::ContextTag(kFFieldId))
        {
            TLV::TLVType outer1;
            size_t destBufSize = f.size();
            uint32_t i         = 0;

            ReturnErrorOnFailure(reader.EnterContainer(outer1));

            while ((err = reader.Next()) == CHIP_NO_ERROR)
            {
                VerifyOrReturnError(destBufSize, CHIP_ERROR_BUFFER_TOO_SMALL);
                ReturnErrorOnFailure(ClusterObjectUtils::Decode(reader, f.data()[i]));
                destBufSize--;
            }

            ReturnErrorOnFailure(reader.ExitContainer(outer1));
        }
    }

    ReturnErrorOnFailure(reader.ExitContainer(outer));
    return CHIP_NO_ERROR;
}
} // namespace NestedStructList

namespace IteratableNestedStructList {
CHIP_ERROR Type::Encode(TLV::TLVWriter & writer, uint64_t tag)
{
    return CHIP_ERROR_BAD_REQUEST;
}

CHIP_ERROR Type::Decode(TLV::TLVReader & reader)
{
    CHIP_ERROR err;
    TLV::TLVType outer;

    err = reader.EnterContainer(outer);
    ReturnErrorOnFailure(err);

    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        uint64_t tag = reader.GetTag();

        if (tag == TLV::ContextTag(kAFieldId))
        {
            ReturnErrorOnFailure(ClusterObjectUtils::Decode(reader, a));
        }
        else if (tag == TLV::ContextTag(kBFieldId))
        {
            ReturnErrorOnFailure(ClusterObjectUtils::Decode(reader, b));
        }
        else if (tag == TLV::ContextTag(kCFieldId))
        {
            ReturnErrorOnFailure(ClusterObjectUtils::Decode(reader, c));
        }
        else if (tag == TLV::ContextTag(kDFieldId))
        {
            ReturnErrorOnFailure(ClusterObjectUtils::Decode(reader, d));
        }
        else if (tag == TLV::ContextTag(kEFieldId))
        {
            ReturnErrorOnFailure(ClusterObjectUtils::Decode(reader, e));
        }
        else if (tag == TLV::ContextTag(kFFieldId))
        {
            ReturnErrorOnFailure(ClusterObjectUtils::Decode(reader, f));
        }
    }

    ReturnErrorOnFailure(reader.ExitContainer(outer));
    return CHIP_NO_ERROR;
}
} // namespace IteratableNestedStructList

namespace DoubleNestedStructList {
CHIP_ERROR Type::Encode(TLV::TLVWriter & writer, uint64_t tag)
{
    TLV::TLVType outer;

    ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));

    {
        TLV::TLVType outer1;

        ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(kAFieldId), TLV::kTLVType_Array, outer1));
        for (auto & item : a)
        {
            ReturnErrorOnFailure(ClusterObjectUtils::Encode(writer, TLV::AnonymousTag, item));
        }
        ReturnErrorOnFailure(writer.EndContainer(outer1));
    }

    ReturnErrorOnFailure(writer.EndContainer(outer));

    return CHIP_NO_ERROR;
}

CHIP_ERROR Type::Decode(TLV::TLVReader & reader)
{
    CHIP_ERROR err;
    TLV::TLVType outer;

    err = reader.EnterContainer(outer);
    ReturnErrorOnFailure(err);

    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        uint64_t tag = reader.GetTag();

        if (tag == TLV::ContextTag(kAFieldId))
        {
            TLV::TLVType outer1;
            size_t destBufSize = a.size();
            uint32_t i         = 0;

            ReturnErrorOnFailure(reader.EnterContainer(outer1));

            while ((err = reader.Next()) == CHIP_NO_ERROR)
            {
                VerifyOrReturnError(destBufSize, CHIP_ERROR_BUFFER_TOO_SMALL);
                ReturnErrorOnFailure(ClusterObjectUtils::Decode(reader, a.data()[i]));
                destBufSize--;
            }

            ReturnErrorOnFailure(reader.ExitContainer(outer1));
        }
    }

    ReturnErrorOnFailure(reader.ExitContainer(outer));
    return CHIP_NO_ERROR;
}
} // namespace DoubleNestedStructList

namespace IteratableDoubleNestedStructList {
CHIP_ERROR Type::Encode(TLV::TLVWriter & writer, uint64_t tag)
{
    return CHIP_ERROR_BAD_REQUEST;
}

CHIP_ERROR Type::Decode(TLV::TLVReader & reader)
{
    CHIP_ERROR err;
    TLV::TLVType outer;

    err = reader.EnterContainer(outer);
    ReturnErrorOnFailure(err);

    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        uint64_t tag = reader.GetTag();

        if (tag == TLV::ContextTag(kAFieldId))
        {
            ReturnErrorOnFailure(ClusterObjectUtils::Decode(reader, a));
        }
    }

    ReturnErrorOnFailure(reader.ExitContainer(outer));
    return CHIP_NO_ERROR;
}
} // namespace IteratableDoubleNestedStructList
} // namespace TestCluster
} // namespace clusters
} // namespace app
} // namespace chip
